/* 
 * File:   request.cpp
 * Author: alex
 * 
 * Created on June 2, 2016, 5:16 PM
 */

#include "server/request.hpp"

#include <cctype>
#include <algorithm>
#include <atomic>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "staticlib/config.hpp"
#include "staticlib/io.hpp"
#include "staticlib/pion.hpp"
#include "staticlib/pion/http_parser.hpp"
#include "staticlib/mustache.hpp"
#include "staticlib/pimpl/forward_macros.hpp"
#include "staticlib/json.hpp"
#include "staticlib/tinydir.hpp"
#include "staticlib/utils.hpp"

#include "wilton/support/exception.hpp"

#include "call/wiltoncall_internal.hpp"

#include "server/response_stream_sender.hpp"
#include "server/request_payload_handler.hpp"
#include "server/server.hpp"

#include "serverconf/header.hpp"
#include "serverconf/response_metadata.hpp"
#include "serverconf/request_metadata.hpp"

namespace wilton {
namespace server {

namespace { // anonymous

using partmap_type = const std::map<std::string, std::string>&;

const std::unordered_set<std::string> HEADERS_DISCARD_DUPLICATES{
    "age", "authorization", "content-length", "content-type", "etag", "expires",
    "from", "host", "if-modified-since", "if-unmodified-since", "last-modified", "location",
    "max-forwards", "proxy-authorization", "referer", "retry-after", "user-agent"
};

} //namespace

class request::impl : public sl::pimpl::object::impl {

    enum class request_state {
        created, committed
    };
    std::atomic<const request_state> state;
    // owning ptrs here to not restrict clients async ops
    sl::pion::http_request_ptr req;
    sl::pion::http_response_writer_ptr resp;
    const std::map<std::string, std::string>& mustache_partials;

public:

    impl(void* /* sl::pion::http_request_ptr&& */ req, void* /* sl::pion::http_response_writer_ptr&& */ resp,
            const std::map<std::string, std::string>& mustache_partials) :
    state(request_state::created),
    req(std::move(*static_cast<sl::pion::http_request_ptr*>(req))),
    resp(std::move(*static_cast<sl::pion::http_response_writer_ptr*> (resp))),
    mustache_partials(mustache_partials) { }

    serverconf::request_metadata get_request_metadata(request&) {
        std::string http_ver = sl::support::to_string(req->get_version_major()) +
                "." + sl::support::to_string(req->get_version_minor());
        auto headers = get_request_headers(*req);
        auto queries = get_queries(*req);
        std::string protocol = resp->get_connection()->get_ssl_flag() ? "https" : "http";
        return serverconf::request_metadata(http_ver, protocol, req->get_method(), req->get_resource(),
                req->get_query_string(), std::move(queries), std::move(headers));
    }

    const std::string& get_request_data(request&) {
        return request_payload_handler::get_data_string(req);
    }

    sl::json::value get_request_form_data(request&) {
        const std::string& data = request_payload_handler::get_data_string(req);
        auto dict = std::unordered_multimap<std::string, std::string, sl::pion::algorithm::ihash, sl::pion::algorithm::iequal_to>();
        auto err = sl::pion::http_parser::parse_url_encoded(dict, data);
        if (!err) throw support::exception(TRACEMSG(
                "Error parsing request body as 'application/x-www-form-urlencoded'"));
        auto res = std::vector<sl::json::field>();
        for (auto& en : dict) {
            res.emplace_back(en.first, en.second);
        }
        return sl::json::value(std::move(res));
    }

    const std::string& get_request_data_filename(request&) {
        return request_payload_handler::get_data_filename(req);
    }

    void set_response_metadata(request&, serverconf::response_metadata rm) {
        resp->get_response().set_status_code(rm.statusCode);
        resp->get_response().set_status_message(rm.statusMessage);
        for (const serverconf::header& ha : rm.headers) {
            resp->get_response().change_header(ha.name, ha.value);
        }
    }

    void send_response(request&, const char* data, uint32_t data_len) {
        if (!state.compare_exchange_strong(request_state::created, request_state::committed)) throw support::exception(TRACEMSG(
                "Invalid request lifecycle operation, request is already committed"));
        resp->write(data, data_len);
        resp->send();
    }

    void send_file(request&, std::string file_path, std::function<void(bool)> finalizer) {
        auto fd = sl::tinydir::file_source(file_path);
        if (!state.compare_exchange_strong(request_state::created, request_state::committed)) throw support::exception(TRACEMSG(
                "Invalid request lifecycle operation, request is already committed"));
        auto fd_ptr = std::unique_ptr<std::streambuf>(sl::io::make_unbuffered_istreambuf_ptr(std::move(fd)));
        auto sender = std::make_shared<response_stream_sender>(resp, std::move(fd_ptr), std::move(finalizer));
        sender->send();
    }

    void send_mustache(request&, std::string mustache_file_path, sl::json::value json) {
        if (!state.compare_exchange_strong(request_state::created, request_state::committed)) throw support::exception(TRACEMSG(
                "Invalid request lifecycle operation, request is already committed"));
        std::string mpath = [&mustache_file_path] () -> std::string {
            if (sl::utils::starts_with(mustache_file_path, internal::file_proto_prefix)) {
                return mustache_file_path.substr(internal::file_proto_prefix.length());
            }
            return mustache_file_path;
        } ();
        auto mp = sl::mustache::source(mpath, std::move(json), mustache_partials);
        auto mp_ptr = std::unique_ptr<std::streambuf>(sl::io::make_unbuffered_istreambuf_ptr(std::move(mp)));
        auto sender = std::make_shared<response_stream_sender>(resp, std::move(mp_ptr));
        sender->send();
    }
    
    response_writer send_later(request&) {
        if (!state.compare_exchange_strong(request_state::created, request_state::committed)) throw support::exception(TRACEMSG(
                "Invalid request lifecycle operation, request is already committed"));
        sl::pion::http_response_writer_ptr writer = this->resp;
        return response_writer{static_cast<void*>(std::addressof(writer))};
    }

    void finish(request&) {
        if (state.compare_exchange_strong(request_state::created, request_state::committed)) {
            resp->send();
        }
    }

private:
    // todo: cookies
    // Duplicates in raw headers are handled in the following ways, depending on the header name:
    // Duplicates of age, authorization, content-length, content-type, etag, expires, 
    // from, host, if-modified-since, if-unmodified-since, last-modified, location, 
    // max-forwards, proxy-authorization, referer, retry-after, or user-agent are discarded.
    // For all other headers, the values are joined together with ', '.
    std::vector<serverconf::header> get_request_headers(sl::pion::http_request& req) {
        std::unordered_map<std::string, serverconf::header> map{};
        for (const auto& en : req.get_headers()) {
            auto ha = serverconf::header(en.first, en.second);
            std::string key = en.first;
            std::transform(key.begin(), key.end(), key.begin(), ::tolower);
            auto inserted = map.emplace(key, std::move(ha));
            if (!inserted.second && 0 == HEADERS_DISCARD_DUPLICATES.count(key)) {
                append_with_comma(inserted.first->second.value, en.second);
            }
        }
        std::vector<serverconf::header> res{};
        for (auto& en : map) {
            res.emplace_back(std::move(en.second));
        }
        std::sort(res.begin(), res.end(), [](const serverconf::header& el1, const serverconf::header & el2) {
            return el1.name < el2.name;
        });
        return res;
    }
    
    std::vector<std::pair<std::string, std::string>> get_queries(sl::pion::http_request& req) {
        std::unordered_map<std::string, std::string> map{};
        for (const auto& en : req.get_queries()) {
            auto inserted = map.emplace(en.first, en.second);
            if (!inserted.second) {
                append_with_comma(inserted.first->second, en.second);
            }
        }
        std::vector<std::pair<std::string, std::string>> res{};
        for (auto& en : map) {
            res.emplace_back(en.first, en.second);
        }
        return res;
    }

    void append_with_comma(std::string& str, const std::string& tail) {
        if (str.empty()) {
            str = tail;
        } else if (!tail.empty()) {
            str.push_back(',');
            str.append(tail);
        }
    }

};
PIMPL_FORWARD_CONSTRUCTOR(request, (void*)(void*)(partmap_type), (), support::exception)
PIMPL_FORWARD_METHOD(request, serverconf::request_metadata, get_request_metadata, (), (), support::exception)
PIMPL_FORWARD_METHOD(request, const std::string&, get_request_data, (), (), support::exception)
PIMPL_FORWARD_METHOD(request, sl::json::value, get_request_form_data, (), (), support::exception)
PIMPL_FORWARD_METHOD(request, const std::string&, get_request_data_filename, (), (), support::exception)
PIMPL_FORWARD_METHOD(request, void, set_response_metadata, (serverconf::response_metadata), (), support::exception)
PIMPL_FORWARD_METHOD(request, void, send_response, (const char*)(uint32_t), (), support::exception)
PIMPL_FORWARD_METHOD(request, void, send_file, (std::string)(std::function<void(bool)>), (), support::exception)
PIMPL_FORWARD_METHOD(request, void, send_mustache, (std::string)(sl::json::value), (), support::exception)
PIMPL_FORWARD_METHOD(request, response_writer, send_later, (), (), support::exception)
PIMPL_FORWARD_METHOD(request, void, finish, (), (), support::exception)

} // namespace
}


/* 
 * File:   Request.cpp
 * Author: alex
 * 
 * Created on June 2, 2016, 5:16 PM
 */

#include "server/Request.hpp"

#include <cctype>
#include <algorithm>
#include <atomic>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "staticlib/config.hpp"
#include "staticlib/io.hpp"
#include "staticlib/httpserver.hpp"
#include "staticlib/pimpl/pimpl_forward_macros.hpp"
#include "staticlib/serialization.hpp"
#include "staticlib/utils.hpp"

#include "mustache/MustacheProcessor.hpp"
#include "ResponseStreamSender.hpp"
#include "StringPayloadHandler.hpp"
#include "common/WiltonInternalException.hpp"

#include "serverconf/Header.hpp"
#include "serverconf/ResponseMetadata.hpp"
#include "serverconf/RequestMetadata.hpp"

namespace wilton {
namespace server {

namespace { // anonymous

namespace sc = staticlib::config;
namespace si = staticlib::io;
namespace sh = staticlib::httpserver;
namespace ss = staticlib::serialization;
namespace su = staticlib::utils;

const std::unordered_set<std::string> HEADERS_DISCARD_DUPLICATES{
    "age", "authorization", "content-length", "content-type", "etag", "expires",
    "from", "host", "if-modified-since", "if-unmodified-since", "last-modified", "location",
    "max-forwards", "proxy-authorization", "referer", "retry-after", "user-agent"
};

} //namespace

class Request::Impl : public staticlib::pimpl::PimplObject::Impl {

    enum class State {
        CREATED, CHUNK_SENT, COMMITTED
    };
    std::atomic<const State> state;
    // owning ptrs here to not restrict clients async ops
    sh::http_request_ptr req;
    sh::http_response_writer_ptr resp;

public:

    Impl(void* /* sh::http_request_ptr&& */ req, void* /* sh::http_response_writer_ptr&& */ resp) :
    state(State::CREATED),
    req(std::move(*static_cast<sh::http_request_ptr*>(req))),
    resp(std::move(*static_cast<sh::http_response_writer_ptr*> (resp))) { }

    serverconf::RequestMetadata get_request_metadata(Request&) {
        std::string http_ver = sc::to_string(req->get_version_major()) +
                "." + sc::to_string(req->get_version_minor());
        auto headers = get_request_headers(*req);
        std::string protocol = resp->get_connection()->get_ssl_flag() ? "https" : "http";
        return serverconf::RequestMetadata(http_ver, protocol, req->get_method(), req->get_resource(),
                req->get_query_string(), std::move(headers));
    }

    const std::string& get_request_data(Request&) {
        return StringPayloadHandler::get_payload(req);
    }

    void set_response_metadata(Request&, serverconf::ResponseMetadata rm) {
        resp->get_response().set_status_code(rm.statusCode);
        resp->get_response().set_status_message(rm.statusMessage);
        for (const serverconf::Header& ha : rm.headers) {
            resp->get_response().change_header(ha.name, ha.value);
        }
    }

    void send_response(Request&, const char* data, uint32_t data_len) {
        if (!state.compare_exchange_strong(State::CREATED, State::COMMITTED)) throw common::WiltonInternalException(TRACEMSG(std::string() +
                "Invalid request lifecycle operation, request is already committed"));
        resp->write(data, data_len);
        resp->send();
    }

    void send_file(Request&, std::string file_path, std::function<void(bool) > finalizer) {
        if (!state.compare_exchange_strong(State::CREATED, State::COMMITTED)) throw common::WiltonInternalException(TRACEMSG(std::string() +
                "Invalid request lifecycle operation, request is already committed"));
        su::FileDescriptor fd{file_path, 'r'};
        auto fd_ptr = si::make_source_istream_ptr(std::move(fd));
        auto sender = std::make_shared<ResponseStreamSender>(resp, std::move(fd_ptr), std::move(finalizer));
        sender->send();
    }

    void send_mustache(Request&, std::string mustache_file_path, ss::JsonValue json) {
        if (!state.compare_exchange_strong(State::CREATED, State::COMMITTED)) throw common::WiltonInternalException(TRACEMSG(std::string() +
                "Invalid request lifecycle operation, request is already committed"));
        auto mp = mustache::MustacheProcessor{mustache_file_path, std::move(json)};
        auto mp_ptr = si::make_source_istream_ptr(std::move(mp));
        auto sender = std::make_shared<ResponseStreamSender>(resp, std::move(mp_ptr));
        sender->send();
    }

    void finish(Request&) {
        if (state.compare_exchange_strong(State::CREATED, State::COMMITTED)) {
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
    std::vector<serverconf::Header> get_request_headers(sh::http_request& req) {
        std::unordered_map<std::string, serverconf::Header> map{};
        for (const auto& en : req.get_headers()) {
            auto ha = serverconf::Header{en.first, en.second};
            std::string key = en.first;
            std::transform(key.begin(), key.end(), key.begin(), ::tolower);
            auto inserted = map.emplace(key, std::move(ha));
            if (!inserted.second && 0 == HEADERS_DISCARD_DUPLICATES.count(key)) {
                append_with_comma(inserted.first->second.value, en.second);
            }
        }
        std::vector<serverconf::Header> res{};
        for (auto& en : map) {
            res.emplace_back(std::move(en.second));
        }
        std::sort(res.begin(), res.end(), [](const serverconf::Header& el1, const serverconf::Header & el2) {
            return el1.name < el2.name;
        });
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
PIMPL_FORWARD_CONSTRUCTOR(Request, (void*)(void*), (), common::WiltonInternalException)
PIMPL_FORWARD_METHOD(Request, serverconf::RequestMetadata, get_request_metadata, (), (), common::WiltonInternalException)
PIMPL_FORWARD_METHOD(Request, const std::string&, get_request_data, (), (), common::WiltonInternalException)
PIMPL_FORWARD_METHOD(Request, void, set_response_metadata, (serverconf::ResponseMetadata), (), common::WiltonInternalException)
PIMPL_FORWARD_METHOD(Request, void, send_response, (const char*)(uint32_t), (), common::WiltonInternalException)
PIMPL_FORWARD_METHOD(Request, void, send_file, (std::string)(std::function<void(bool)>), (), common::WiltonInternalException)
PIMPL_FORWARD_METHOD(Request, void, send_mustache, (std::string)(ss::JsonValue), (), common::WiltonInternalException)
PIMPL_FORWARD_METHOD(Request, void, finish, (), (), common::WiltonInternalException)

} // namespace
}


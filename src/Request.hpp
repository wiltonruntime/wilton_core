/* 
 * File:   Request.hpp
 * Author: alex
 *
 * Created on May 5, 2016, 12:30 PM
 */

#ifndef WILTON_C_REQUEST_HPP
#define	WILTON_C_REQUEST_HPP

#include <cctype>
#include <algorithm>
#include <atomic>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "staticlib/config.hpp"
#include "staticlib/httpserver.hpp"

#include "StringPayloadHandler.hpp"
#include "WiltonInternalException.hpp"

#include "json/Header.hpp"
#include "json/ResponseMetadata.hpp"
#include "json/RequestMetadata.hpp"

namespace wilton {
namespace c {

namespace { // anonymous

namespace sc = staticlib::config;
namespace sh = staticlib::httpserver;

const std::unordered_set<std::string> HEADERS_DISCARD_DUPLICATES{
        "age", "authorization", "content-length", "content-type", "etag", "expires", 
        "from", "host", "if-modified-since", "if-unmodified-since", "last-modified", "location", 
        "max-forwards", "proxy-authorization", "referer", "retry-after", "user-agent"};

} //namespace

class Request {
    enum class State { CREATED, CHUNK_SENT, COMMITTED };
    std::atomic<const State> state;
    // owning ptrs here to not restrict clients async ops
    sh::http_request_ptr req;
    sh::http_response_writer_ptr resp;
    
public:
    Request(sh::http_request_ptr&& req, sh::http_response_writer_ptr&& resp) :
    state(State::CREATED),
    req(std::move(req)),
    resp(std::move(resp)) { }

    json::RequestMetadata get_request_metadata() {
        std::string http_ver = sc::to_string(req->get_version_major()) + 
                "." + sc::to_string(req->get_version_minor());
        auto headers = get_request_headers(*req);
        return json::RequestMetadata(http_ver, req->get_method(), req->get_resource(), 
                req->get_query_string(), std::move(headers));
    }
    
    const std::string& get_request_data() {
        return StringPayloadHandler::get_payload(req);
    }
    
    void set_response_metadata(json::ResponseMetadata rm) {
        resp->get_response().set_status_code(rm.statusCode);
        resp->get_response().set_status_message(rm.statusMessage);
        for (const json::Header& ha : rm.headers) {
            resp->get_response().change_header(ha.name, ha.value);
        }
    }
    
    void send_response(const char* data, uint32_t data_len) {
        if (!state.compare_exchange_strong(State::CREATED, State::COMMITTED)) throw WiltonInternalException(TRACEMSG(std::string() +
                "Invalid request lifecycle operation, request is already committed"));
        resp->write(data, data_len);
        resp->send();
    }
    
    void finish() {
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
    std::vector<json::Header> get_request_headers(sh::http_request& req) {
        std::unordered_map<std::string, json::Header> map{};
        for (const auto& en : req.get_headers()) {
            auto ha = json::Header{en.first, en.second};
            std::string key = en.first;
            std::transform(key.begin(), key.end(), key.begin(), ::tolower);
            auto inserted = map.emplace(key, std::move(ha));
            if (!inserted.second && 0 == HEADERS_DISCARD_DUPLICATES.count(key)) {
                append_with_comma(inserted.first->second.value, en.second);
            }
        }
        std::vector<json::Header> res{};
        for (auto& en : map) {
            res.emplace_back(std::move(en.second));
        }
        std::sort(res.begin(), res.end(), [](const json::Header& el1, const json::Header& el2) {
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

} // namespace
}

#endif	/* WILTON_C_REQUEST_HPP */


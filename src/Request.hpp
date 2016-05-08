/* 
 * File:   Request.hpp
 * Author: alex
 *
 * Created on May 5, 2016, 12:30 PM
 */

#ifndef WILTON_C_REQUEST_HPP
#define	WILTON_C_REQUEST_HPP

#include <atomic>

#include "staticlib/config.hpp"
#include "staticlib/httpserver.hpp"

#include "ResponseMetadata.hpp"
#include "RequestMetadata.hpp"
#include "StringPayloadHandler.hpp"
#include "WiltonInternalException.hpp"

namespace wilton {
namespace c {

namespace { // anonymous

namespace sc = staticlib::config;
namespace sh = staticlib::httpserver;

}

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

    RequestMetadata get_request_metadata() {
        std::string http_ver = sc::to_string(req->get_version_major()) + 
                "." + sc::to_string(req->get_version_minor());
        return RequestMetadata(http_ver, req->get_method(), req->get_resource(), 
                req->get_query_string());
    }
    
    const std::string& get_request_data() {
        return StringPayloadHandler::get_payload(req);
    }
    
    void set_response_metadata(ResponseMetadata rm) {
        resp->get_response().set_status_code(rm.statusCode);
        resp->get_response().set_status_message(rm.statusMessage);
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
    
};

} // namespace
}

#endif	/* WILTON_C_REQUEST_HPP */


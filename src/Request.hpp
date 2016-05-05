/* 
 * File:   Request.hpp
 * Author: alex
 *
 * Created on May 5, 2016, 12:30 PM
 */

#ifndef WILTON_C_REQUEST_HPP
#define	WILTON_C_REQUEST_HPP

#include "staticlib/httpserver/http_request.hpp"
#include "staticlib/httpserver/http_response_writer.hpp"

#include "staticlib/config.hpp"

#include "ResponseMetadata.hpp"
#include "RequestMetadata.hpp"
#include "StringPayloadHandler.hpp"

namespace wilton {
namespace c {

namespace { // anonymous

namespace sc = staticlib::config;
namespace sh = staticlib::httpserver;

}

class Request {
    // owning ptrs here to not restrict clients async ops
    sh::http_request_ptr req;
    sh::http_response_writer_ptr resp;
    
public:
    Request(sh::http_request_ptr&& req, sh::http_response_writer_ptr&& resp) :
    req(std::move(req)),
    resp(std::move(resp)) { }

    RequestMetadata get_request_metadata() {
        return RequestMetadata();
    }
    
    std::string& get_request_data() {
        return StringPayloadHandler::get_payload(req);
    }
    
    // todo: medatada validation
    void set_response_metadata(ResponseMetadata rm) {
        
    }
    
    void send_response(const char* data, uint32_t data_len) {
//        if (!state.compare_exchange_strong(State::CREATED, State::COMMITTED)) throw CBindHttpException(TRACEMSG(std::string() +
//                "Invalid request lifecycle operation, request is already committed"));
        resp->write(data, data_len);
        resp->send();
    }
    
};

} // namespace
}

#endif	/* WILTON_C_REQUEST_HPP */


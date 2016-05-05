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
#include "staticlib/serialization.hpp"

#include "StringPayloadHandler.hpp"

namespace wilton {
namespace c {

namespace { // anonymous

namespace sc = staticlib::config;
namespace ss = staticlib::serialization;
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

    ss::JsonValue get_request_metadata() {
        return {"httpMethod", req.get_method() };
    }
    
    std::string& get_request_data() {
        return StringPayloadHandler::get_payload(req);
    }
    
    // todo: medatada validation
    void set_response_metadata(ss::JsonValue&& json) {
        for (const auto& fi : json.get_object()) {
            auto name = fi.get_name();
            if ("statusCode" == name) {
                resp.get_response().set_status_code(fi.get_uint32());
            } else if ("statusMessage" == name) {
                resp.get_response().set_status_code(fi.get_string());
            } else if ("headers" == name) {
                for (const auto& he : json.get_object()) {
                    // todo
                }
            }
        }
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


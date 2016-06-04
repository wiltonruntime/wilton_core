/* 
 * File:   Request.hpp
 * Author: alex
 *
 * Created on May 5, 2016, 12:30 PM
 */

#ifndef WILTON_SERVER_REQUEST_HPP
#define	WILTON_SERVER_REQUEST_HPP

#include <cstdint>
#include <string>
#include <functional>

#include "staticlib/pimpl.hpp"
#include "staticlib/serialization.hpp"

#include "common/WiltonInternalException.hpp"
#include "json/ResponseMetadata.hpp"
#include "json/RequestMetadata.hpp"

namespace wilton {
namespace server {

class Request : public staticlib::pimpl::PimplObject {
protected:
    /**
     * Implementation class
     */
    class Impl;
public:
    /**
     * PIMPL-specific constructor
     * 
     * @param pimpl impl object
     */
    PIMPL_CONSTRUCTOR(Request)
    
    json::RequestMetadata get_request_metadata();
    
    const std::string& get_request_data();
    
    void set_response_metadata(json::ResponseMetadata rm);
    
    void send_response(const char* data, uint32_t data_len);
    
    void send_file(std::string file_path, std::function<void(bool)> finalizer);
    
    void send_mustache(std::string mustache_file_path, staticlib::serialization::JsonValue json);
    
    void finish();
    
    // private api
    
    Request(void* /* staticlib::httpserver::http_request_ptr&& */ req, 
            void* /* staticlib::httpserver::http_response_writer_ptr&& */ resp);
};

} // namespace
}

#endif	/* WILTON_SERVER_REQUEST_HPP */


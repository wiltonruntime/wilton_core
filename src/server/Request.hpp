/* 
 * File:   Request.hpp
 * Author: alex
 *
 * Created on May 5, 2016, 12:30 PM
 */

#ifndef WILTON_SERVER_REQUEST_HPP
#define	WILTON_SERVER_REQUEST_HPP

#include <cstdint>
#include <functional>
#include <map>
#include <string>

#include "staticlib/pimpl.hpp"
#include "staticlib/serialization.hpp"

#include "common/WiltonInternalException.hpp"
#include "server/ResponseWriter.hpp"
#include "serverconf/ResponseMetadata.hpp"
#include "serverconf/RequestMetadata.hpp"

namespace wilton {
namespace server {

class Request : public staticlib::pimpl::pimpl_object {
protected:
    /**
     * implementation class
     */
    class impl;
public:
    /**
     * PIMPL-specific constructor
     * 
     * @param pimpl impl object
     */
    PIMPL_CONSTRUCTOR(Request)
    
    serverconf::RequestMetadata get_request_metadata();
    
    const std::string& get_request_data();
    
    const std::string& get_request_data_filename();
    
    void set_response_metadata(serverconf::ResponseMetadata rm);
    
    void send_response(const char* data, uint32_t data_len);
    
    void send_file(std::string file_path, std::function<void(bool)> finalizer);
    
    void send_mustache(std::string mustache_file_path, staticlib::serialization::json_value json);
    
    ResponseWriter send_later();
    
    void finish();
    
    // private api
    
    Request(void* /* staticlib::httpserver::http_request_ptr&& */ req, 
            void* /* staticlib::httpserver::http_response_writer_ptr&& */ resp,
            const std::map<std::string, std::string>& mustache_partials);
};

} // namespace
}

#endif	/* WILTON_SERVER_REQUEST_HPP */


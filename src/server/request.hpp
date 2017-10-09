/* 
 * File:   request.hpp
 * Author: alex
 *
 * Created on May 5, 2016, 12:30 PM
 */

#ifndef WILTON_SERVER_REQUEST_HPP
#define WILTON_SERVER_REQUEST_HPP

#include <cstdint>
#include <functional>
#include <map>
#include <string>

#include "staticlib/pimpl.hpp"
#include "staticlib/json.hpp"

#include "wilton/support/exception.hpp"

#include "server/response_writer.hpp"
#include "serverconf/response_metadata.hpp"
#include "serverconf/request_metadata.hpp"

namespace wilton {
namespace server {

class request : public sl::pimpl::object {
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
    PIMPL_CONSTRUCTOR(request)
    
    serverconf::request_metadata get_request_metadata();
    
    const std::string& get_request_data();

    sl::json::value get_request_form_data();
    
    const std::string& get_request_data_filename();
    
    void set_response_metadata(serverconf::response_metadata rm);
    
    void send_response(const char* data, uint32_t data_len);
    
    void send_file(std::string file_path, std::function<void(bool)> finalizer);
    
    void send_mustache(std::string mustache_file_path, sl::json::value json);
    
    response_writer send_later();
    
    void finish();
    
    // private api
    
    request(void* /* sl::pion::http_request_ptr&& */ req, 
            void* /* sl::pion::http_response_writer_ptr&& */ resp,
            const std::map<std::string, std::string>& mustache_partials);
};

} // namespace
}

#endif /* WILTON_SERVER_REQUEST_HPP */


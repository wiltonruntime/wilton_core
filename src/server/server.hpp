/* 
 * File:   HttpServer.hpp
 * Author: alex
 *
 * Created on May 5, 2016, 12:30 PM
 */

#ifndef WILTON_SERVER_SERVER_HPP
#define WILTON_SERVER_SERVER_HPP

#include <vector>

#include "staticlib/pimpl.hpp"

#include "server/http_path.hpp"
#include "server/request.hpp"
#include "serverconf/server_config.hpp"

namespace wilton {
namespace server {

class server : public sl::pimpl::object {
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
    PIMPL_CONSTRUCTOR(server)
            
    server(serverconf::server_config conf, std::vector<sl::support::observer_ptr<http_path>> paths);
    
    void stop();
    
};

} // namespace
}

#endif /* WILTON_SERVER_SERVER_HPP */


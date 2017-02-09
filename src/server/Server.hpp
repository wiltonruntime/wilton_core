/* 
 * File:   HttpServer.hpp
 * Author: alex
 *
 * Created on May 5, 2016, 12:30 PM
 */

#ifndef WILTON_SERVER_SERVER_HPP
#define	WILTON_SERVER_SERVER_HPP

#include <functional>
#include <vector>

#include "staticlib/pimpl.hpp"

#include "server/HttpPath.hpp"
#include "server/Request.hpp"
#include "serverconf/ServerConfig.hpp"

namespace wilton {
namespace server {

class Server : public staticlib::pimpl::pimpl_object {
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
    PIMPL_CONSTRUCTOR(Server)
            
    Server(serverconf::ServerConfig conf, std::vector<std::reference_wrapper<HttpPath>> paths);
    
    void stop();
    
};

} // namespace
}

#endif	/* WILTON_SERVER_SERVER_HPP */


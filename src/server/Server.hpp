/* 
 * File:   HttpServer.hpp
 * Author: alex
 *
 * Created on May 5, 2016, 12:30 PM
 */

#ifndef WILTON_SERVER_SERVER_HPP
#define	WILTON_SERVER_SERVER_HPP

#include <functional>

#include "staticlib/pimpl.hpp"

#include "server/Request.hpp"
#include "json/ServerConfig.hpp"

namespace wilton {
namespace server {

class Server : public staticlib::pimpl::PimplObject {
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
    PIMPL_CONSTRUCTOR(Server)
            
    Server(std::function<void(Request& req)> gateway, json::ServerConfig conf);
    
    void stop();
    
};

} // namespace
}

#endif	/* WILTON_SERVER_SERVER_HPP */


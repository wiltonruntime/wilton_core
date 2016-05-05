/* 
 * File:   Server.hpp
 * Author: alex
 *
 * Created on May 5, 2016, 12:30 PM
 */

#ifndef WILTON_C_SERVER_HPP
#define	WILTON_C_SERVER_HPP

#include <funtional>
#include <memory>
#include <string>

#include "staticlib/httpserver/http_request.hpp"
#include "staticlib/httpserver/http_response_writer.hpp"
#include "staticlib/httpserver/http_server.hpp"
#include "staticlib/httpserver/tcp_connection.hpp"

#include "Request.hpp"
#include "ServerConfig.hpp"

namespace wilton {
namespace c {

namespace { // anonymous

namespace sc = staticlib::config;
namespace sh = staticlib::httpserver;

}

class Server {
    std::unique_ptr<sh::http_server> server;
    
    Server(std::function<void(Request& req)> handler, ServerConfig config) {
        // todo create server, apply config
    }
    
    void stop() {
        server->stop();
    }
    
};

} // namespace
}

#endif	/* WILTON_C_SERVER_HPP */


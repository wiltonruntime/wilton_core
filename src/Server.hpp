/* 
 * File:   Server.hpp
 * Author: alex
 *
 * Created on May 5, 2016, 12:30 PM
 */

#ifndef WILTON_C_SERVER_HPP
#define	WILTON_C_SERVER_HPP

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "staticlib/config.hpp"
#include "staticlib/httpserver.hpp"

#include "Request.hpp"
#include "ServerConfig.hpp"
#include "StringPayloadHandler.hpp"

namespace wilton {
namespace c {

namespace { // anonymous

namespace sc = staticlib::config;
namespace sh = staticlib::httpserver;

}

class Server {
    std::unique_ptr<sh::http_server> server;
    
public:    
    Server(std::function<void(Request& req)> handler, ServerConfig config) :
    server(sc::make_unique<sh::http_server>(config.numberOfThreads, config.tcpPort)) {
        std::vector<std::string> methods = {"GET", "POST", "PUT", "DELETE"};
        std::string path = "/";
        for (const std::string& me : methods) {
            server->add_payload_handler(me, path, StringPayloadHandler::create);
            server->add_handler(me, path,
                    [handler](sh::http_request_ptr& req, sh::tcp_connection_ptr& conn) {
                        auto finfun = std::bind(&sh::tcp_connection::finish, conn);
                        auto writer = sh::http_response_writer::create(conn, *req, finfun);
                        Request req_pass{std::move(req), std::move(writer)};
                        handler(req_pass);
                        req_pass.finish();
                    });
        }
        server->start();
    }
    
    void stop() {
        server->stop();
    }
    
};

} // namespace
}

#endif	/* WILTON_C_SERVER_HPP */


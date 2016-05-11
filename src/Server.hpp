/* 
 * File:   HttpServer.hpp
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
#include "staticlib/serialization.hpp"

#include "FileHandler.hpp"
#include "Request.hpp"
#include "StringPayloadHandler.hpp"
#include "WiltonInternalException.hpp"
#include "WiltonLogger.hpp"
#include "ZipHandler.hpp"

#include "json/ServerConfig.hpp"
#include "json/Logging.hpp"

namespace wilton {
namespace c {

namespace { // anonymous

namespace sc = staticlib::config;
namespace sh = staticlib::httpserver;
namespace ss = staticlib::serialization;

}

class Server {
    std::unique_ptr<sh::http_server> server;
    
public:    
    Server(std::function<void(Request& req)> gateway, json::ServerConfig conf) :
    server(sc::make_unique<sh::http_server>(conf.numberOfThreads, conf.tcpPort)) {
        WiltonLogger::apply_config(conf.logging);
        std::vector<std::string> methods = {"GET", "POST", "PUT", "DELETE"};
        std::string path = "/";
        for (const std::string& me : methods) {
            server->add_payload_handler(me, path, StringPayloadHandler::create);
            server->add_handler(me, path,
                    [gateway](sh::http_request_ptr& req, sh::tcp_connection_ptr& conn) {
                        auto finfun = std::bind(&sh::tcp_connection::finish, conn);
                        auto writer = sh::http_response_writer::create(conn, *req, finfun);
                        Request req_pass{std::move(req), std::move(writer)};
                        gateway(req_pass);
                        req_pass.finish();
                    });
        }
        for (const auto& dr : conf.documentRoots) {
            if (dr.dirPath.length() > 0) {
                server->add_handler("GET", dr.resource, FileHandler(dr));
            } else if (dr.zipPath.length() > 0) {
                server->add_handler("GET", dr.resource, ZipHandler(dr));
            } else throw WiltonInternalException(TRACEMSG(std::string() + 
                    "invalid 'documentRoot': [" + ss::to_json_string(dr) + "]"));
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


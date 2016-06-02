/* 
 * File:   Server.cpp
 * Author: alex
 * 
 * Created on June 2, 2016, 5:33 PM
 */

#include "server/Server.hpp"

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "staticlib/config.hpp"
#include "staticlib/httpserver.hpp"
#include "staticlib/pimpl/pimpl_forward_macros.hpp"
#include "staticlib/serialization.hpp"

#include "FileHandler.hpp"
#include "Request.hpp"
#include "StringPayloadHandler.hpp"
#include "WiltonInternalException.hpp"
#include "logging/WiltonLogger.hpp"
#include "ZipHandler.hpp"

#include "json/ServerConfig.hpp"
#include "json/Logging.hpp"

namespace wilton {
namespace server {

namespace { // anonymous

namespace sc = staticlib::config;
namespace sh = staticlib::httpserver;
namespace ss = staticlib::serialization;

typedef std::function<void(Request& req)> gateway_fun_type;

}

class Server::Impl : public staticlib::pimpl::PimplObject::Impl {
    std::unique_ptr<sh::http_server> server;

public:
    Impl(gateway_fun_type gateway, json::ServerConfig conf) :
    server(sc::make_unique<sh::http_server>(conf.numberOfThreads, conf.tcpPort)) {
        logging::WiltonLogger::apply_config(conf.logging);
        std::vector<std::string> methods = {"GET", "POST", "PUT", "DELETE"};
        std::string path = "/";
        for (const std::string& me : methods) {
            server->add_payload_handler(me, path, StringPayloadHandler::create);
            server->add_handler(me, path,
                    [gateway](sh::http_request_ptr& req, sh::tcp_connection_ptr & conn) {
                        auto finfun = std::bind(&sh::tcp_connection::finish, conn);
                        auto writer = sh::http_response_writer::create(conn, *req, finfun);
                                Request req_pass{static_cast<void*> (std::addressof(req)),
                            static_cast<void*> (std::addressof(writer))};
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
                    "Invalid 'documentRoot': [" + ss::dump_json_to_string(dr.to_json()) + "]"));
        }
        server->start();
    }

    void stop(Server&) {
        server->stop();
    }
    
};
PIMPL_FORWARD_CONSTRUCTOR(Server, (gateway_fun_type)(json::ServerConfig), (), WiltonInternalException)
PIMPL_FORWARD_METHOD(Server, void, stop, (), (), WiltonInternalException)

} // namespace
}

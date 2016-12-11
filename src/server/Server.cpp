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

#include "asio.hpp"

#include "staticlib/config.hpp"
#include "staticlib/httpserver.hpp"
#include "staticlib/io.hpp"
#include "staticlib/pimpl/pimpl_forward_macros.hpp"
#include "staticlib/serialization.hpp"
#include "staticlib/tinydir.hpp"
#include "staticlib/utils.hpp"

#include "common/WiltonInternalException.hpp"
#include "logging/WiltonLogger.hpp"
#include "server/FileHandler.hpp"
#include "server/Request.hpp"
#include "server/RequestPayloadHandler.hpp"
#include "server/ZipHandler.hpp"

#include "logging/LoggingConfig.hpp"
#include "serverconf/ServerConfig.hpp"

namespace wilton {
namespace server {

namespace { // anonymous

namespace sc = staticlib::config;
namespace sh = staticlib::httpserver;
namespace si = staticlib::io;
namespace ss = staticlib::serialization;
namespace st = staticlib::tinydir;
namespace su = staticlib::utils;

using partmap_type = const std::map<std::string, std::string>&;

} // namespace

class Server::Impl : public staticlib::pimpl::PimplObject::Impl {
    std::map<std::string, std::string> mustache_partials;
    std::unique_ptr<sh::http_server> server;

public:
    Impl(serverconf::ServerConfig conf, std::vector<std::reference_wrapper<HttpPath>> paths) :
    mustache_partials(load_partials(conf.mustache)),
    server(sc::make_unique<sh::http_server>(
            conf.numberOfThreads, 
            conf.tcpPort,
            asio::ip::address_v4::from_string(conf.ipAddress),
            conf.ssl.keyFile,
            create_pwd_cb(conf.ssl.keyPassword),
            conf.ssl.verifyFile,
            create_verifier_cb(conf.ssl.verifySubjectSubstr))) {
        logging::WiltonLogger::apply_config(conf.logging);
        auto conf_ptr = std::make_shared<serverconf::RequestPayloadConfig>(conf.requestPayload.clone());
        for (const HttpPath& pa : paths) {
            auto ha = pa.handler; // copy
            server->add_handler(pa.method, pa.path,
                    [ha, this](sh::http_request_ptr& req, sh::tcp_connection_ptr & conn) {
                        auto writer = sh::http_response_writer::create(conn, req);
                        Request req_wrap{static_cast<void*> (std::addressof(req)),
                                static_cast<void*> (std::addressof(writer)), this->mustache_partials};
                        ha(req_wrap);
                        req_wrap.finish();
                    });
            server->add_payload_handler(pa.method, pa.path, [conf_ptr](staticlib::httpserver::http_request_ptr& /* request */) {
                return RequestPayloadHandler{*conf_ptr};
            });
        }
        for (const auto& dr : conf.documentRoots) {
            if (dr.dirPath.length() > 0) {
                server->add_handler("GET", dr.resource, FileHandler(dr));
            } else if (dr.zipPath.length() > 0) {
                server->add_handler("GET", dr.resource, ZipHandler(dr));
            } else throw common::WiltonInternalException(TRACEMSG(
                    "Invalid 'documentRoot': [" + ss::dump_json_to_string(dr.to_json()) + "]"));
        }        
        server->start();
    }

    void stop(Server&) {
        server->stop();
    }
    
private:
    static std::function<std::string(std::size_t, asio::ssl::context::password_purpose) > create_pwd_cb(const std::string& password) {
        return [password](std::size_t, asio::ssl::context::password_purpose) {
            return password;
        };
    }

    static std::string extract_subject(asio::ssl::verify_context& ctx) {
        if (ctx.native_handle() && ctx.native_handle()->current_cert && ctx.native_handle()->current_cert->name) {
            return std::string(ctx.native_handle()->current_cert->name);
        } else return "";
    }

    static std::function<bool(bool, asio::ssl::verify_context&)> create_verifier_cb(const std::string& subject_part) {
        return [subject_part](bool preverify_ok, asio::ssl::verify_context & ctx) {
            // cert validation fail
            if (!preverify_ok) {
                return false;
            }
            // not the leaf certificate
            if (ctx.native_handle()->error_depth > 0) {
                return true;
            }
            // no subject restrictions
            if (subject_part.empty()) {
                return true;
            }
            // check substr
            std::string subject = extract_subject(ctx);
            auto pos = subject.find(subject_part);
            return std::string::npos != pos;
        };
    }
    
    static std::map<std::string, std::string> load_partials(const serverconf::MustacheConfig& cf) {
        static std::string MUSTACHE_EXT = ".mustache";
        std::map<std::string, std::string> res;
        for (const std::string& dirpath : cf.partialsDirs) {
            for (const st::TinydirFile& tf : st::list_directory(dirpath)) {
                if (!su::ends_with(tf.get_name(), MUSTACHE_EXT)) continue;
                std::string name = std::string(tf.get_name().data(), tf.get_name().length() - MUSTACHE_EXT.length());
                std::string val = read_file(tf);
                auto pa = res.insert(std::make_pair(std::move(name), std::move(val)));
                if (!pa.second) throw common::WiltonInternalException(TRACEMSG(
                        "Invalid duplicate 'mustache.partialsDirs' element," +
                        " dirpath: [" + dirpath + "], path: [" + tf.get_path() + "]"));
            }
        }
        return res;
    }

    static std::string read_file(const st::TinydirFile& tf) {
        auto fd = tf.open_read();
        std::array<char, 4096> buf;
        si::string_sink sink{};
        si::copy_all(fd, sink, buf.data(), buf.size());
        return std::move(sink.get_string());
    }
    
};
PIMPL_FORWARD_CONSTRUCTOR(Server, (serverconf::ServerConfig)(std::vector<std::reference_wrapper<HttpPath>>), (), common::WiltonInternalException)
PIMPL_FORWARD_METHOD(Server, void, stop, (), (), common::WiltonInternalException)

} // namespace
}

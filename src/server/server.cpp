/* 
 * File:   server.cpp
 * Author: alex
 * 
 * Created on June 2, 2016, 5:33 PM
 */

#include "server/server.hpp"

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "asio.hpp"

#include "staticlib/config.hpp"
#include "staticlib/pion.hpp"
#include "staticlib/io.hpp"
#include "staticlib/pimpl/forward_macros.hpp"
#include "staticlib/tinydir.hpp"
#include "staticlib/utils.hpp"

#include "call/wiltoncall_internal.hpp"
#include "common/wilton_internal_exception.hpp"
#include "logging/wilton_logger.hpp"
#include "server/file_handler.hpp"
#include "server/request.hpp"
#include "server/request_payload_handler.hpp"
#include "server/zip_handler.hpp"

#include "logging/logging_config.hpp"
#include "serverconf/server_config.hpp"

namespace wilton {
namespace server {

namespace { // anonymous

using partmap_type = const std::map<std::string, std::string>&;

} // namespace

class server::impl : public sl::pimpl::object::impl {
    std::map<std::string, std::string> mustache_partials;
    std::unique_ptr<sl::pion::http_server> server_ptr;

public:
    impl(serverconf::server_config conf, std::vector<sl::support::observer_ptr<http_path>> paths) :
    mustache_partials(load_partials(conf.mustache)),
    server_ptr(std::unique_ptr<sl::pion::http_server>(new sl::pion::http_server(
            conf.numberOfThreads, 
            conf.tcpPort,
            asio::ip::address_v4::from_string(conf.ipAddress),
            conf.ssl.keyFile,
            create_pwd_cb(conf.ssl.keyPassword),
            conf.ssl.verifyFile,
            create_verifier_cb(conf.ssl.verifySubjectSubstr)))) {
        auto conf_ptr = std::make_shared<serverconf::request_payload_config>(conf.requestPayload.clone());
        for (auto& pa : paths) {
            auto ha = pa->handler; // copy
            server_ptr->add_handler(pa->method, pa->path,
                    [ha, this](sl::pion::http_request_ptr& req, sl::pion::tcp_connection_ptr& conn) {
                        auto writer = sl::pion::http_response_writer::create(conn, req);
                        request req_wrap{static_cast<void*> (std::addressof(req)),
                                static_cast<void*> (std::addressof(writer)), this->mustache_partials};
                        ha(req_wrap);
                        req_wrap.finish();
                    });
            server_ptr->add_payload_handler(pa->method, pa->path, [conf_ptr](sl::pion::http_request_ptr& /* request */) {
                return request_payload_handler{*conf_ptr};
            });
        }
        for (const auto& dr : conf.documentRoots) {
            if (dr.dirPath.length() > 0) {
                server_ptr->add_handler("GET", dr.resource, file_handler(dr));
            } else if (dr.zipPath.length() > 0) {
                server_ptr->add_handler("GET", dr.resource, zip_handler(dr));
            } else throw common::wilton_internal_exception(TRACEMSG(
                    "Invalid 'documentRoot': [" + dr.to_json().dumps() + "]"));
        }
        server_ptr->get_scheduler().set_thread_stop_hook([](const std::thread::id& tid) STATICLIB_NOEXCEPT {
            wilton::internal::clean_duktape_thread_local(tid);
        });
        server_ptr->start();
    }

    void stop(server&) {
        server_ptr->stop();
    }
    
private:
    static std::function<std::string(std::size_t, asio::ssl::context::password_purpose)> create_pwd_cb(const std::string& password) {
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
    
    static std::map<std::string, std::string> load_partials(const serverconf::mustache_config& cf) {
        static std::string MUSTACHE_EXT = ".mustache";
        std::map<std::string, std::string> res;
        for (const std::string& dirpath : cf.partialsDirs) {
            for (const sl::tinydir::path& tf : sl::tinydir::list_directory(dirpath)) {
                if (!sl::utils::ends_with(tf.filename(), MUSTACHE_EXT)) continue;
                std::string name = std::string(tf.filename().data(), tf.filename().length() - MUSTACHE_EXT.length());
                std::string val = read_file(tf);
                auto pa = res.insert(std::make_pair(std::move(name), std::move(val)));
                if (!pa.second) throw common::wilton_internal_exception(TRACEMSG(
                        "Invalid duplicate 'mustache.partialsDirs' element," +
                        " dirpath: [" + dirpath + "], path: [" + tf.filepath() + "]"));
            }
        }
        return res;
    }

    static std::string read_file(const sl::tinydir::path& tf) {
        auto fd = tf.open_read();
        sl::io::string_sink sink{};
        sl::io::copy_all(fd, sink);
        return std::move(sink.get_string());
    }
    
};
PIMPL_FORWARD_CONSTRUCTOR(server, (serverconf::server_config)(std::vector<sl::support::observer_ptr<http_path>>), (), common::wilton_internal_exception)
PIMPL_FORWARD_METHOD(server, void, stop, (), (), common::wilton_internal_exception)

} // namespace
}

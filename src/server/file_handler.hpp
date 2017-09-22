/* 
 * File:   file_handler.hpp
 * Author: alex
 *
 * Created on May 11, 2016, 12:50 PM
 */

#ifndef WILTON_SERVER_FILE_HANDLER_HPP
#define WILTON_SERVER_FILE_HANDLER_HPP

#include <cstdint>
#include <memory>
#include <streambuf>
#include <string>

#include "staticlib/config.hpp"
#include "staticlib/io.hpp"
#include "staticlib/pion.hpp"
#include "staticlib/tinydir.hpp"
#include "staticlib/utils.hpp"

#include "wilton/support/exception.hpp"

#include "response_stream_sender.hpp"
#include "serverconf/document_root.hpp"

namespace wilton {
namespace server {

class file_handler {
    std::shared_ptr<serverconf::document_root> conf;
    
public:
    // must be copyable to satisfy std::function
    file_handler(const file_handler& other) :
    conf(other.conf) { }

    file_handler& operator=(const file_handler& other) {
        this->conf = other.conf;
        return *this;
    }

    // todo: path leading slash check
    file_handler(const serverconf::document_root& conf) :
    conf(std::make_shared<serverconf::document_root>(conf.clone())) {
        if (0 == this->conf->dirPath.length()) throw support::exception(TRACEMSG(
                "Invalid empty 'dirPath' specified"));
    }
    
    // todo: error messages format
    // todo: path checks
    void operator()(sl::pion::http_request_ptr& req, sl::pion::tcp_connection_ptr& conn) {
        auto resp = sl::pion::http_response_writer::create(conn, req);
        std::string url_path = std::string{req->get_resource(), conf->resource.length()};
        if (url_path.find("..") != std::string::npos) {
            resp->get_response().set_status_code(sl::pion::http_request::RESPONSE_CODE_BAD_REQUEST);
            resp->get_response().set_status_message(sl::pion::http_request::RESPONSE_MESSAGE_BAD_REQUEST);
            resp << sl::pion::http_request::RESPONSE_CODE_BAD_REQUEST << " "
                    << sl::pion::http_request::RESPONSE_MESSAGE_BAD_REQUEST << ":"
                    << " [" << url_path << "]\n";
            resp->send();
        } else {
            try {
                std::string file_path = std::string(conf->dirPath) +"/" + url_path;
                auto fd = sl::tinydir::file_source(file_path);
                auto fd_ptr = std::unique_ptr<std::streambuf>(sl::io::make_unbuffered_istreambuf_ptr(std::move(fd)));
                auto sender = std::make_shared<response_stream_sender>(resp, std::move(fd_ptr));
                set_resp_headers(url_path, resp->get_response());
                sender->send();
            } catch (const std::exception&) {
                resp->get_response().set_status_code(sl::pion::http_request::RESPONSE_CODE_NOT_FOUND);
                resp->get_response().set_status_message(sl::pion::http_request::RESPONSE_MESSAGE_NOT_FOUND);
                resp << sl::pion::http_request::RESPONSE_CODE_NOT_FOUND << " "
                        << sl::pion::http_request::RESPONSE_MESSAGE_NOT_FOUND << ":"
                        << " [" << url_path << "]\n";
                resp->send();
            }
        }
    }
    
private:
    void set_resp_headers(const std::string& url_path, sl::pion::http_response& resp) {
        std::string ct{"application/octet-stream"};
        for(const auto& mi : conf->mimeTypes) {
            if (sl::utils::ends_with(url_path, mi.extension)) {
                ct = mi.mime;
                break;
            }
        }
        resp.change_header("Content-Type", ct);
        // set caching
        resp.change_header("Cache-Control", "max-age=" + sl::support::to_string(conf->cacheMaxAgeSeconds) + ", public");
    }
    
};

} // namespace
}

#endif /* WILTON_SERVER_FILE_HANDLER_HPP */


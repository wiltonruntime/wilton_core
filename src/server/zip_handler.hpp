/* 
 * File:   zip_handler.hpp
 * Author: alex
 *
 * Created on May 11, 2016, 12:50 PM
 */

#ifndef WILTON_SERVER_ZIP_HANDLER_HPP
#define	WILTON_SERVER_ZIP_HANDLER_HPP

#include <cstdint>
#include <memory>
#include <string>

#include "staticlib/config.hpp"
#include "staticlib/io.hpp"
#include "staticlib/unzip.hpp"

#include "response_stream_sender.hpp"
#include "common/wilton_internal_exception.hpp"

#include "serverconf/document_root.hpp"

namespace wilton {
namespace server {

namespace { // anonymous

namespace sc = staticlib::config;
namespace sh = staticlib::httpserver;
namespace uz = staticlib::unzip;
namespace su = staticlib::utils;

} // namespace

class zip_handler {
    std::shared_ptr<serverconf::document_root> conf;
    std::shared_ptr<uz::unzip_file_index> idx;
    
public:
    // must be copyable to satisfy std::function
    zip_handler(const zip_handler& other) :
    conf(other.conf),
    idx(other.idx) { }

    zip_handler& operator=(const zip_handler& other) {
        this->conf = other.conf;
        this->idx = other.idx;
        return *this;
    }
    
    zip_handler(const serverconf::document_root& conf) :
    conf(std::make_shared<serverconf::document_root>(conf.clone())),
    idx(std::make_shared<uz::unzip_file_index>(conf.zipPath)) { }    
    
    // todo: error messages format
    // todo: path checks
    void operator()(sh::http_request_ptr& req, sh::tcp_connection_ptr& conn) {
        auto resp = sh::http_response_writer::create(conn, req);
        std::string url_path = conf->zipInnerPrefix + std::string{req->get_resource(), conf->resource.length()};
        uz::file_entry en = idx->find_zip_entry(url_path);        
        if (!en.is_empty()) {
            auto stream_ptr = uz::open_zip_entry(*idx, url_path);
            auto sender = std::make_shared<response_stream_sender>(resp, std::move(stream_ptr));
            set_resp_headers(url_path, resp->get_response());
            sender->send();
        } else {
            resp->get_response().set_status_code(sh::http_request::RESPONSE_CODE_NOT_FOUND);
            resp->get_response().set_status_message(sh::http_request::RESPONSE_MESSAGE_NOT_FOUND);
            resp << sh::http_request::RESPONSE_CODE_NOT_FOUND << " "
                    << sh::http_request::RESPONSE_MESSAGE_NOT_FOUND << ":"
                    << " [" << url_path << "]\n";
            resp->send();
        }
    }

private:
    void set_resp_headers(const std::string& url_path, sh::http_response& resp) {
        std::string ct{"application/octet-stream"};
        for (const auto& mi : conf->mimeTypes) {
            if (su::ends_with(url_path, mi.extension)) {
                ct = mi.mime;
                break;
            }
        }
        resp.change_header("Content-Type", ct);
        // set caching
        resp.change_header("Cache-Control", "max-age=" + sc::to_string(conf->cacheMaxAgeSeconds) + ", public");
    }    
    
};

} // namespace
}

#endif	/* WILTON_SERVER_ZIP_HANDLER_HPP */

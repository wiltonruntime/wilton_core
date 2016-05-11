/* 
 * File:   ZipHandler.hpp
 * Author: alex
 *
 * Created on May 11, 2016, 12:50 PM
 */

#ifndef WILTON_C_ZIPHANDLER_HPP
#define	WILTON_C_ZIPHANDLER_HPP

#include <cstdint>
#include <string>

#include "staticlib/config.hpp"
#include "staticlib/io.hpp"
#include "staticlib/httpserver.hpp"
#include "staticlib/unzip.hpp"

#include "ResponseStreamSender.hpp"
#include "WiltonInternalException.hpp"

#include "json/DocumentRoot.hpp"

namespace wilton {
namespace c {

namespace { // anonymous

namespace sc = staticlib::config;
namespace sh = staticlib::httpserver;
namespace uz = staticlib::unzip;
namespace su = staticlib::utils;

} // namespace

class ZipHandler {
    json::DocumentRoot conf;
    uz::UnzipFileIndex idx;
    
public:
    ZipHandler(const json::DocumentRoot& conf) :
    idx(conf.zipPath),
    conf(conf.clone()) { }    
    
    // todo: error messages format
    void operator()(sh::http_request_ptr& req, sh::tcp_connection_ptr& conn) {
        auto finfun = std::bind(&sh::tcp_connection::finish, conn);
        auto resp = sh::http_response_writer::create(conn, *req, finfun);
        std::string url_path = std::string{req->get_resource(), conf.resource.length()};
        uz::FileEntry en = idx.find_zip_entry(url_path);        
        if (!en.is_empty()) {
            auto stream_ptr = uz::open_zip_entry(idx, url_path);
            auto sender = std::make_shared<ResponseStreamSender>(resp, std::move(stream_ptr));
            set_resp_headers(resp->get_response());
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
        for (const auto& mi : conf.mimeTypes) {
            if (su::ends_with(url_path, mi.extension)) {
                ct = mi.mime;
                break;
            }
        }
        resp.change_header("Content-Type", ct);
        // set caching
        resp.change_header("Cache-Control", "max-age=" + sc::to_string(conf.cacheMaxAgeSeconds) + ", public");
    }    
    
};

} // namespace
}


#endif	/* WILTON_C_ZIPHANDLER_HPP */


/* 
 * File:   wilton_client.cpp
 * Author: alex
 * 
 * Created on June 13, 2016, 4:23 PM
 */

#include "wilton/wilton.h"

#include <array>
#include <string>

#include "staticlib/config.hpp"
#include "staticlib/http.hpp"
#include "staticlib/io.hpp"
#include "staticlib/json.hpp"
#include "staticlib/utils.hpp"
#include "staticlib/tinydir.hpp"

#include "client/client_response.hpp"
#include "client/client_request_config.hpp"
#include "client/client_session_config.hpp"

struct wilton_HttpClient {
private:
    sl::http::multi_threaded_session delegate;

public:
    wilton_HttpClient(sl::http::multi_threaded_session&& delegate) :
    delegate(std::move(delegate)) { }

    sl::http::session& impl() {
        return delegate;
    }
};

char* wilton_HttpClient_create(
        wilton_HttpClient** http_out,
        const char* conf_json,
        int conf_json_len) {
    if (nullptr == http_out) return sl::utils::alloc_copy(TRACEMSG("Null 'http_out' parameter specified"));
    if (nullptr == conf_json) return sl::utils::alloc_copy(TRACEMSG("Null 'conf_json' parameter specified"));
    if (!sl::support::is_uint32_positive(conf_json_len)) return sl::utils::alloc_copy(TRACEMSG(
            "Invalid 'conf_json_len' parameter specified: [" + sl::support::to_string(conf_json_len) + "]"));
    try {
        uint32_t conf_json_len_u32 = static_cast<uint32_t> (conf_json_len);
        std::string json_str{conf_json, conf_json_len_u32};
        sl::json::value json = sl::json::loads(json_str);
        wilton::client::client_session_config conf{std::move(json)};
        sl::http::multi_threaded_session session{std::move(conf.options)};
        wilton_HttpClient* http_ptr = new wilton_HttpClient(std::move(session));
        *http_out = http_ptr;
        return nullptr;
    } catch (const std::exception& e) {
        return sl::utils::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

char* wilton_HttpClient_close(
        wilton_HttpClient* http) {
    if (nullptr == http) return sl::utils::alloc_copy(TRACEMSG("Null 'http' parameter specified"));
    try {
        delete http;
        return nullptr;
    } catch (const std::exception& e) {
        return sl::utils::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

char* wilton_HttpClient_execute(
        wilton_HttpClient* http,
        const char* url,
        int url_len,
        const char* request_data,
        int request_data_len,
        const char* request_metadata_json,
        int request_metadata_len,
        char** response_data_out,
        int* response_data_len_out) {
    if (nullptr == http) return sl::utils::alloc_copy(TRACEMSG("Null 'http' parameter specified"));    
    if (nullptr == url) return sl::utils::alloc_copy(TRACEMSG("Null 'url' parameter specified"));
    if (!sl::support::is_uint32_positive(url_len)) return sl::utils::alloc_copy(TRACEMSG(
            "Invalid 'url_len' parameter specified: [" + sl::support::to_string(url_len) + "]"));
    if (!sl::support::is_uint32(request_data_len)) return sl::utils::alloc_copy(TRACEMSG(
            "Invalid 'request_data_len' parameter specified: [" + sl::support::to_string(request_data_len) + "]"));
    if (!sl::support::is_uint32(request_metadata_len)) return sl::utils::alloc_copy(TRACEMSG(
            "Invalid 'request_metadata_len' parameter specified: [" + sl::support::to_string(request_metadata_len) + "]"));
    try {
        std::string url_str{url, static_cast<uint32_t> (url_len)};
        sl::json::value opts_json{};
        if (request_metadata_len > 0) {
            opts_json = sl::json::load({request_metadata_json, request_metadata_len});
        }
        wilton::client::client_request_config opts{std::move(opts_json)};
        std::array<char, 4096> buf;
        sl::io::string_sink sink{};
        sl::json::value resp_json{};
        if (request_data_len > 0) {
            auto data_src = sl::io::array_source(request_data, static_cast<uint32_t> (request_data_len));
            // POST will be used by default for this API call
            sl::http::resource resp = http->impl().open_url(url_str, std::move(data_src), opts.options);            
            sl::io::copy_all(resp, sink, buf);
            resp_json = wilton::client::client_response::to_json(std::move(sink.get_string()), resp, resp.get_info());
        } else {
            // GET will be used by default for this API call
            sl::http::resource resp = http->impl().open_url(url_str, opts.options);
            sl::io::copy_all(resp, sink, buf);
            resp_json = wilton::client::client_response::to_json(std::move(sink.get_string()), resp, resp.get_info());
        }
        std::string resp_complete = resp_json.dumps();
        *response_data_out = sl::utils::alloc_copy(resp_complete);
        *response_data_len_out = static_cast<int>(resp_complete.length());
        return nullptr;
    } catch (const std::exception& e) {
        return sl::utils::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }    
}

char* wilton_HttpClient_send_file(
        wilton_HttpClient* http,
        const char* url,
        int url_len,
        const char* file_path,
        int file_path_len,
        const char* request_metadata_json,
        int request_metadata_len,
        char** response_data_out,
        int* response_data_len_out,
        void* finalizer_ctx,
        void (*finalizer_cb)(
                void* finalizer_ctx,
                int sent_successfully)) {
    if (nullptr == http) return sl::utils::alloc_copy(TRACEMSG("Null 'http' parameter specified"));
    if (nullptr == url) return sl::utils::alloc_copy(TRACEMSG("Null 'url' parameter specified"));
    if (!sl::support::is_uint32_positive(url_len)) return sl::utils::alloc_copy(TRACEMSG(
            "Invalid 'url_len' parameter specified: [" + sl::support::to_string(url_len) + "]"));
    if (nullptr == file_path) return sl::utils::alloc_copy(TRACEMSG("Null 'file_path' parameter specified"));
    if (!sl::support::is_uint16_positive(file_path_len)) return sl::utils::alloc_copy(TRACEMSG(
            "Invalid 'file_path_len' parameter specified: [" + sl::support::to_string(file_path_len) + "]"));
    if (!sl::support::is_uint32(request_metadata_len)) return sl::utils::alloc_copy(TRACEMSG(
            "Invalid 'request_metadata_len' parameter specified: [" + sl::support::to_string(request_metadata_len) + "]"));
    try {
        std::string url_str{url, static_cast<uint32_t> (url_len)};
        sl::json::value opts_json{};
        if (request_metadata_len > 0) {
            std::string meta_str{request_metadata_json, static_cast<uint32_t> (request_metadata_len)};
            opts_json = sl::json::loads(meta_str);
        }
        wilton::client::client_request_config opts{std::move(opts_json)};
        std::array<char, 4096> buf;
        std::string file_path_str{file_path, static_cast<uint32_t> (file_path_len)};
        auto fd = sl::tinydir::file_source(file_path_str);
        sl::http::resource resp = http->impl().open_url(url_str, std::move(fd), opts.options);
        sl::io::string_sink sink{};
        sl::io::copy_all(resp, sink, buf);
        sl::json::value resp_json = wilton::client::client_response::to_json(
                std::move(sink.get_string()), resp, resp.get_info());
        std::string resp_complete = resp_json.dumps();
        if (nullptr != finalizer_cb) {
            finalizer_cb(finalizer_ctx, 1);
        }
        *response_data_out = sl::utils::alloc_copy(resp_complete);
        *response_data_len_out = static_cast<int>(resp_complete.length());
        return nullptr;
    } catch (const std::exception& e) {
        if (nullptr != finalizer_cb) {
            finalizer_cb(finalizer_ctx, 0);
        }
        return sl::utils::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }   
}

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
#include "staticlib/httpclient.hpp"
#include "staticlib/io.hpp"
#include "staticlib/serialization.hpp"
#include "staticlib/utils.hpp"

#include "client/ClientResponse.hpp"
#include "client/ClientRequestConfig.hpp"
#include "client/ClientSessionConfig.hpp"

namespace { // anonymous

namespace sc = staticlib::config;
namespace sh = staticlib::httpclient;
namespace si = staticlib::io;
namespace ss = staticlib::serialization;
namespace su = staticlib::utils;
namespace wc = wilton::client;
    
} //namespace

struct wilton_HttpClient {
private:
    sh::HttpSession delegate;

public:
    wilton_HttpClient(sh::HttpSession&& delegate) :
    delegate(std::move(delegate)) { }

    sh::HttpSession& impl() {
        return delegate;
    }
};

char* wilton_HttpClient_create(
        wilton_HttpClient** http_out,
        const char* conf_json,
        int conf_json_len) {
    if (nullptr == http_out) return su::alloc_copy(TRACEMSG("Null 'http_out' parameter specified"));
    if (nullptr == conf_json) return su::alloc_copy(TRACEMSG("Null 'conf_json' parameter specified"));
    if (!su::is_positive_uint32(conf_json_len)) return su::alloc_copy(TRACEMSG(
            "Invalid 'conf_json_len' parameter specified: [" + sc::to_string(conf_json_len) + "]"));
    try {
        uint32_t conf_json_len_u32 = static_cast<uint32_t> (conf_json_len);
        std::string json_str{conf_json, conf_json_len_u32};
        ss::JsonValue json = ss::load_json_from_string(json_str);
        wc::ClientSessionConfig conf{std::move(json)};
        sh::HttpSession session{std::move(conf.options)};
        wilton_HttpClient* http_ptr = new wilton_HttpClient(std::move(session));
        *http_out = http_ptr;
        return nullptr;
    } catch (const std::exception& e) {
        return su::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

char* wilton_HttpClient_close(
        wilton_HttpClient* http) {
    if (nullptr == http) return su::alloc_copy(TRACEMSG("Null 'http' parameter specified"));
    try {
        delete http;
        return nullptr;
    } catch (const std::exception& e) {
        return su::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

// todo: json copying
// todo: data copying
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
    if (nullptr == http) return su::alloc_copy(TRACEMSG("Null 'http' parameter specified"));    
    if (nullptr == url) return su::alloc_copy(TRACEMSG("Null 'url' parameter specified"));
    if (!su::is_positive_uint32(url_len)) return su::alloc_copy(TRACEMSG(
            "Invalid 'url_len' parameter specified: [" + sc::to_string(url_len) + "]"));
    if (!su::is_uint32(request_data_len)) return su::alloc_copy(TRACEMSG(
            "Invalid 'request_data_len' parameter specified: [" + sc::to_string(request_data_len) + "]"));
    if (!su::is_uint32(request_metadata_len)) return su::alloc_copy(TRACEMSG(
            "Invalid 'request_metadata_len' parameter specified: [" + sc::to_string(request_metadata_len) + "]"));
    try {
        std::string url_str{url, static_cast<uint32_t> (url_len)};
        ss::JsonValue opts_json{};
        if (request_metadata_len > 0) {
            std::string meta_str{request_metadata_json, static_cast<uint32_t> (request_metadata_len)};
            opts_json = ss::load_json_from_string(meta_str);
        }
        wc::ClientRequestConfig opts{std::move(opts_json)};
        std::array<char, 4096> buf;
        si::string_sink sink{};
        ss::JsonValue resp_json{};
        if (request_data_len > 0) {
            std::string data_str{request_data, static_cast<uint32_t> (request_data_len)};
            si::string_source data_src{std::move(data_str)};
            sh::HttpResource resp = http->impl().open_url(url_str, std::move(data_src), opts.options);            
            si::copy_all(resp, sink, buf.data(), buf.size());
            resp_json = wc::ClientResponse::to_json(std::move(sink.get_string()), resp.get_info());
        } else {
            sh::HttpResource resp = http->impl().open_url(url_str, opts.options);
            si::copy_all(resp, sink, buf.data(), buf.size());
            resp_json = wc::ClientResponse::to_json(std::move(sink.get_string()), resp.get_info());
        }
        std::string resp_complete = ss::dump_json_to_string(resp_json);
        *response_data_out = su::alloc_copy(resp_complete);
        *response_data_len_out = resp_complete.length();
        return nullptr;
    } catch (const std::exception& e) {
        return su::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
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
    if (nullptr == http) return su::alloc_copy(TRACEMSG("Null 'http' parameter specified"));
    if (nullptr == url) return su::alloc_copy(TRACEMSG("Null 'url' parameter specified"));
    if (!su::is_positive_uint32(url_len)) return su::alloc_copy(TRACEMSG(
            "Invalid 'url_len' parameter specified: [" + sc::to_string(url_len) + "]"));
    if (nullptr == file_path) return su::alloc_copy(TRACEMSG("Null 'file_path' parameter specified"));
    if (!su::is_positive_uint16(file_path_len)) return su::alloc_copy(TRACEMSG(
            "Invalid 'file_path_len' parameter specified: [" + sc::to_string(file_path_len) + "]"));
    if (!su::is_uint32(request_metadata_len)) return su::alloc_copy(TRACEMSG(
            "Invalid 'request_metadata_len' parameter specified: [" + sc::to_string(request_metadata_len) + "]"));
    try {
        std::string url_str{url, static_cast<uint32_t> (url_len)};
        ss::JsonValue opts_json{};
        if (request_metadata_len > 0) {
            std::string meta_str{request_metadata_json, static_cast<uint32_t> (request_metadata_len)};
            opts_json = ss::load_json_from_string(meta_str);
        }
        wc::ClientRequestConfig opts{std::move(opts_json)};
        std::array<char, 4096> buf;
        std::string file_path_str{file_path, static_cast<uint32_t> (file_path_len)};
        su::FileDescriptor fd{file_path_str, 'r'};
        sh::HttpResource resp = http->impl().open_url(url_str, std::move(fd), opts.options);
        si::string_sink sink{};
        si::copy_all(resp, sink, buf.data(), buf.size());
        ss::JsonValue resp_json = wc::ClientResponse::to_json(std::move(sink.get_string()), resp.get_info());
        std::string resp_complete = ss::dump_json_to_string(resp_json);
        if (nullptr != finalizer_cb) {
            finalizer_cb(finalizer_ctx, 1);
        }
        *response_data_out = su::alloc_copy(resp_complete);
        *response_data_len_out = resp_complete.length();
        return nullptr;
    } catch (const std::exception& e) {
        if (nullptr != finalizer_cb) {
            finalizer_cb(finalizer_ctx, 0);
        }
        return su::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }   
}

/* 
 * File:   wiltoncall_client.cpp
 * Author: alex
 *
 * Created on January 10, 2017, 5:40 PM
 */

#include "call/wiltoncall_internal.hpp"

#include <memory>

#include "wilton/wilton.h"

namespace wilton {
namespace client {

namespace { //anonymous

wilton_HttpClient* static_client() {
    static std::string cfg = sl::json::dumps({
        {"multiThreaded", true} 
    });
    static std::unique_ptr<wilton_HttpClient, std::function<void(wilton_HttpClient*)>> client = 
            std::unique_ptr<wilton_HttpClient, std::function<void(wilton_HttpClient*)>>(
            []{
                wilton_HttpClient* http;
                char* err = wilton_HttpClient_create(std::addressof(http), cfg.data(), static_cast<int> (cfg.size()));
                if (nullptr != err) {
                    common::throw_wilton_error(err, TRACEMSG(err));
                }
                return http;
            }(),
            [] (wilton_HttpClient* http) {
                wilton_HttpClient_close(http);
            });
    return client.get();
}

} // namespace

support::buffer httpclient_send_request(sl::io::span<const char> data) {
    // json parse
    auto json = sl::json::load(data);
    auto rurl = std::ref(sl::utils::empty_string());
    auto rdata = std::ref(sl::utils::empty_string());
    std::string metadata = sl::utils::empty_string();
    for (const sl::json::field& fi : json.as_object()) {
        auto& name = fi.name();
        if ("url" == name) {
            rurl = fi.as_string_nonempty_or_throw(name);
        } else if ("data" == name) {
            rdata = fi.as_string();
        } else if ("metadata" == name) {
            metadata = fi.val().dumps();
        } else {
            throw common::wilton_internal_exception(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (rurl.get().empty()) throw common::wilton_internal_exception(TRACEMSG(
            "Required parameter 'url' not specified"));
    const std::string& url = rurl.get();
    const std::string& request_data = rdata.get();
    // call wilton
    char* out = nullptr;
    int out_len = 0;
    char* err = wilton_HttpClient_execute(static_client(), url.c_str(), static_cast<int>(url.length()),
            request_data.c_str(), static_cast<int>(request_data.length()), 
            metadata.c_str(), static_cast<int>(metadata.length()),
            std::addressof(out), std::addressof(out_len));
    if (nullptr != err) common::throw_wilton_error(err, TRACEMSG(err));
    return support::wrap_wilton_buffer(out, out_len);
}

support::buffer httpclient_send_file(sl::io::span<const char> data) {
    // json parse
    auto json = sl::json::load(data);
    auto rurl = std::ref(sl::utils::empty_string());
    auto rfile = std::ref(sl::utils::empty_string());
    std::string metadata = sl::utils::empty_string();
    auto rem = false;
    for (const sl::json::field& fi : json.as_object()) {
        auto& name = fi.name();
        if ("url" == name) {
            rurl = fi.as_string_nonempty_or_throw(name);
        } else if ("filePath" == name) {
            rfile = fi.as_string_nonempty_or_throw(name);
        } else if ("metadata" == name) {
            metadata = fi.val().dumps();
        } else if ("remove" == name) {
            rem = fi.as_bool_or_throw(name);
        } else {
            throw common::wilton_internal_exception(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (rurl.get().empty()) throw common::wilton_internal_exception(TRACEMSG(
            "Required parameter 'url' not specified"));
    if (rfile.get().empty()) throw common::wilton_internal_exception(TRACEMSG(
            "Required parameter 'filePath' not specified"));
    const std::string& url = rurl.get();
    const std::string& file_path = rfile.get();
    // call wilton
    char* out = nullptr;
    int out_len = 0;
    std::string* pass_ctx = rem ? new std::string(file_path.data(), file_path.length()) : new std::string();
    char* err = wilton_HttpClient_send_file(static_client(), url.c_str(), static_cast<int>(url.length()),
            file_path.c_str(), static_cast<int>(file_path.length()), 
            metadata.c_str(), static_cast<int>(metadata.length()),
            std::addressof(out), std::addressof(out_len),
            pass_ctx,
            [](void* ctx, int) {
                std::string* filePath_passed = static_cast<std::string*> (ctx);
                if (!filePath_passed->empty()) {
                    std::remove(filePath_passed->c_str());
                }
                delete filePath_passed;
            });
    if (nullptr != err) common::throw_wilton_error(err, TRACEMSG(err));
    return support::wrap_wilton_buffer(out, out_len);
}

} // namespace
}

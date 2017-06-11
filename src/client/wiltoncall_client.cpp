/* 
 * File:   wiltoncall_client.cpp
 * Author: alex
 *
 * Created on January 10, 2017, 5:40 PM
 */

#include "call/wiltoncall_internal.hpp"

#include "wilton/wilton.h"

namespace wilton {
namespace client {

namespace { //anonymous

common::handle_registry<wilton_HttpClient>& static_registry() {
    static common::handle_registry<wilton_HttpClient> registry;
    return registry;
}

} // namespace

sl::support::optional<sl::io::span<char>> httpclient_create(sl::io::span<const char> data) {
    wilton_HttpClient* http;
    char* err = wilton_HttpClient_create(std::addressof(http), data.data(), static_cast<int>(data.size()));
    if (nullptr != err) common::throw_wilton_error(err, TRACEMSG(err));
    int64_t handle = static_registry().put(http);
    return common::into_span({
        { "httpclientHandle", handle}
    });
}

sl::support::optional<sl::io::span<char>> httpclient_close(sl::io::span<const char> data) {
    // json parse
    auto json = sl::json::load(data);
    int64_t handle = -1;
    for (const sl::json::field& fi : json.as_object()) {
        auto& name = fi.name();
        if ("httpclientHandle" == name) {
            handle = fi.as_int64_or_throw(name);
        } else {
            throw common::wilton_internal_exception(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw common::wilton_internal_exception(TRACEMSG(
            "Required parameter 'httpclientHandle' not specified"));
    // get handle
    wilton_HttpClient* http = static_registry().remove(handle);
    if (nullptr == http) throw common::wilton_internal_exception(TRACEMSG(
            "Invalid 'httpclientHandle' parameter specified"));
    // call wilton
    char* err = wilton_HttpClient_close(http);
    if (nullptr != err) {
        static_registry().put(http);
        common::throw_wilton_error(err, TRACEMSG(err));
    }
    return common::empty_span();
}

sl::support::optional<sl::io::span<char>> httpclient_execute(sl::io::span<const char> data) {
    // json parse
    auto json = sl::json::load(data);
    int64_t handle = -1;
    auto rurl = std::ref(sl::utils::empty_string());
    auto rdata = std::ref(sl::utils::empty_string());
    std::string metadata = sl::utils::empty_string();
    for (const sl::json::field& fi : json.as_object()) {
        auto& name = fi.name();
        if ("httpclientHandle" == name) {
            handle = fi.as_int64_or_throw(name);
        } else if ("url" == name) {
            rurl = fi.as_string_nonempty_or_throw(name);
        } else if ("data" == name) {
            rdata = fi.as_string();
        } else if ("metadata" == name) {
            metadata = fi.val().dumps();
        } else {
            throw common::wilton_internal_exception(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw common::wilton_internal_exception(TRACEMSG(
            "Required parameter 'httpclientHandle' not specified"));
    if (rurl.get().empty()) throw common::wilton_internal_exception(TRACEMSG(
            "Required parameter 'url' not specified"));
    const std::string& url = rurl.get();
    const std::string& request_data = rdata.get();
    // get handle
    wilton_HttpClient* http = static_registry().remove(handle);
    if (nullptr == http) throw common::wilton_internal_exception(TRACEMSG(
            "Invalid 'httpclientHandle' parameter specified"));
    // call wilton
    char* out = nullptr;
    int out_len = 0;
    char* err = wilton_HttpClient_execute(http, url.c_str(), static_cast<int>(url.length()),
            request_data.c_str(), static_cast<int>(request_data.length()), 
            metadata.c_str(), static_cast<int>(metadata.length()),
            std::addressof(out), std::addressof(out_len));
    static_registry().put(http);
    if (nullptr != err) common::throw_wilton_error(err, TRACEMSG(err));
    return common::into_span(out, out_len);
}

sl::support::optional<sl::io::span<char>> httpclient_send_temp_file(sl::io::span<const char> data) {
    // json parse
    auto json = sl::json::load(data);
    int64_t handle = -1;
    auto rurl = std::ref(sl::utils::empty_string());
    auto rfile = std::ref(sl::utils::empty_string());
    std::string metadata = sl::utils::empty_string();
    for (const sl::json::field& fi : json.as_object()) {
        auto& name = fi.name();
        if ("httpclientHandle" == name) {
            handle = fi.as_int64_or_throw(name);
        } else if ("url" == name) {
            rurl = fi.as_string_nonempty_or_throw(name);
        } else if ("filePath" == name) {
            rfile = fi.as_string_nonempty_or_throw(name);
        } else if ("metadata" == name) {
            metadata = fi.val().dumps();
        } else {
            throw common::wilton_internal_exception(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw common::wilton_internal_exception(TRACEMSG(
            "Required parameter 'httpclientHandle' not specified"));
    if (rurl.get().empty()) throw common::wilton_internal_exception(TRACEMSG(
            "Required parameter 'url' not specified"));
    if (rfile.get().empty()) throw common::wilton_internal_exception(TRACEMSG(
            "Required parameter 'filePath' not specified"));
    const std::string& url = rurl.get();
    const std::string& file_path = rfile.get();
    // get handle
    wilton_HttpClient* http = static_registry().remove(handle);
    if (nullptr == http) throw common::wilton_internal_exception(TRACEMSG(
            "Invalid 'httpclientHandle' parameter specified"));
    // call wilton
    char* out = nullptr;
    int out_len = 0;
    char* err = wilton_HttpClient_send_file(http, url.c_str(), static_cast<int>(url.length()),
            file_path.c_str(), static_cast<int>(file_path.length()), 
            metadata.c_str(), static_cast<int>(metadata.length()),
            std::addressof(out), std::addressof(out_len),
            new std::string(file_path.data(), file_path.length()),
            [](void* ctx, int) {
                std::string* filePath_passed = static_cast<std::string*> (ctx);
                std::remove(filePath_passed->c_str());
                delete filePath_passed;
            });
    static_registry().put(http);
    if (nullptr != err) common::throw_wilton_error(err, TRACEMSG(err));
    return common::into_span(out, out_len);
}

} // namespace
}

/* 
 * File:   wilton_client.cpp
 * Author: alex
 * 
 * Created on June 13, 2016, 4:23 PM
 */

#include "wilton/wilton.h"

#include "staticlib/config.hpp"
#include "staticlib/httpclient.hpp"
#include "staticlib/serialization.hpp"
#include "staticlib/utils.hpp"

#include "client/ClientSessionConfig.hpp"

namespace { // anonymous

namespace sc = staticlib::config;
namespace sh = staticlib::httpclient;
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
    if (conf_json_len <= 0 ||
            static_cast<int64_t> (conf_json_len) > std::numeric_limits<uint32_t>::max()) return su::alloc_copy(TRACEMSG(
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

WILTON_EXPORT char* wilton_HttpClient_close(
        wilton_HttpClient* http) {
    if (nullptr == http) return su::alloc_copy(TRACEMSG(
            "Null 'http' parameter specified"));
    try {
        delete http;
        return nullptr;
    } catch (const std::exception& e) {
        return su::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

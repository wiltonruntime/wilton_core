/* 
 * File:   wilton_misc.cpp
 * Author: alex
 * 
 * Created on June 4, 2016, 7:22 PM
 */

#include "wilton/wilton.h"

#include <cstdlib>
#include <thread>
#include <chrono>

#include "staticlib/config.hpp"
#include "staticlib/utils.hpp"
#include "TCPConnectChecker.hpp"

namespace { // anonymous

namespace sc = staticlib::config;
namespace su = staticlib::utils;
namespace wm = wilton::misc;

}

void wilton_free(char* errmsg) /* noexcept */ {
    std::free(errmsg);
}

char* wilton_thread_sleep_millis(int millis) /* noexcept */ {
    if (!su::is_positive_uint32(millis)) return su::alloc_copy(TRACEMSG(
            "Invalid 'millis' parameter specified: [" + sc::to_string(millis) + "]"));
    try {
        uint32_t millis_u32 = static_cast<uint32_t> (millis);
        std::this_thread::sleep_for(std::chrono::milliseconds{millis_u32});
        return nullptr;
    } catch (const std::exception& e) {
        return su::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

char* wilton_tcp_wait_for_connection(const char* ip_addr, int ip_addr_len, 
        int tcp_port, int timeout_millis) /* noexcept */ {
    if (nullptr == ip_addr) return su::alloc_copy(TRACEMSG("Null 'ip_addr' parameter specified"));
    if (!su::is_uint32(ip_addr_len)) return su::alloc_copy(TRACEMSG(
            "Invalid 'ip_addr_len' parameter specified: [" + sc::to_string(ip_addr_len) + "]"));
    if (!su::is_positive_uint16(tcp_port)) return su::alloc_copy(TRACEMSG(
            "Invalid 'tcp_port' parameter specified: [" + sc::to_string(tcp_port) + "]"));
    if (!su::is_positive_uint32(timeout_millis)) return su::alloc_copy(TRACEMSG(
            "Invalid 'timeout_millis' parameter specified: [" + sc::to_string(timeout_millis) + "]"));
    try {
        uint32_t ip_addr_len_u32 = static_cast<uint32_t> (ip_addr_len);
        std::string ip_addr_str{ip_addr, ip_addr_len_u32};
        uint16_t tcp_port_u16 = static_cast<uint16_t> (tcp_port);
        uint32_t timeout_millis_u32 = static_cast<uint32_t> (timeout_millis);
        std::chrono::milliseconds timeout{timeout_millis_u32};
        std::string err = wm::TCPConnectChecker::wait_for_connection(timeout, ip_addr_str, tcp_port_u16);
        if (err.empty()) {
            return nullptr;
        } else {
            return su::alloc_copy(err);
        }
    } catch (const std::exception& e) {
        return su::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

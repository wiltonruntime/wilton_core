/* 
 * File:   wilton_misc.cpp
 * Author: alex
 * 
 * Created on June 4, 2016, 7:22 PM
 */

#include "wilton/wilton.h"

#include <cstdint>
#include <string>

#include "staticlib/config.hpp"
#include "staticlib/utils.hpp"
#include "tcp_connect_checker.hpp"

char* wilton_alloc(int size_bytes) /* noexcept */ {
    if (!sl::support::is_uint32_positive(size_bytes)) {
        return nullptr;
    }
    return reinterpret_cast<char*>(std::malloc(static_cast<size_t>(size_bytes)));
}

void wilton_free(char* errmsg) /* noexcept */ {
    std::free(errmsg);
}

char* wilton_tcp_wait_for_connection(const char* ip_addr, int ip_addr_len, 
        int tcp_port, int timeout_millis) /* noexcept */ {
    if (nullptr == ip_addr) return sl::utils::alloc_copy(TRACEMSG("Null 'ip_addr' parameter specified"));
    if (!sl::support::is_uint32(ip_addr_len)) return sl::utils::alloc_copy(TRACEMSG(
            "Invalid 'ip_addr_len' parameter specified: [" + sl::support::to_string(ip_addr_len) + "]"));
    if (!sl::support::is_uint16_positive(tcp_port)) return sl::utils::alloc_copy(TRACEMSG(
            "Invalid 'tcp_port' parameter specified: [" + sl::support::to_string(tcp_port) + "]"));
    if (!sl::support::is_uint32_positive(timeout_millis)) return sl::utils::alloc_copy(TRACEMSG(
            "Invalid 'timeout_millis' parameter specified: [" + sl::support::to_string(timeout_millis) + "]"));
    try {
        uint32_t ip_addr_len_u32 = static_cast<uint32_t> (ip_addr_len);
        std::string ip_addr_str{ip_addr, ip_addr_len_u32};
        uint16_t tcp_port_u16 = static_cast<uint16_t> (tcp_port);
        uint32_t timeout_millis_u32 = static_cast<uint32_t> (timeout_millis);
        std::chrono::milliseconds timeout{timeout_millis_u32};
        std::string err = wilton::misc::tcp_connect_checker::wait_for_connection(timeout, ip_addr_str, tcp_port_u16);
        if (err.empty()) {
            return nullptr;
        } else {
            return sl::utils::alloc_copy(err);
        }
    } catch (const std::exception& e) {
        return sl::utils::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

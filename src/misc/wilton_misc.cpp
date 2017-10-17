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

#include "wilton/support/alloc_copy.hpp"

#include "call/wiltoncall_internal.hpp"

char* wilton_alloc(int size_bytes) /* noexcept */ {
    if (!sl::support::is_uint32_positive(size_bytes)) {
        return nullptr;
    }
    return reinterpret_cast<char*>(std::malloc(static_cast<size_t>(size_bytes)));
}

void wilton_free(char* buffer) /* noexcept */ {
    std::free(buffer);
}

char* wilton_clean_tls(const char* thread_id, int thread_id_len) {
    if (nullptr == thread_id) return wilton::support::alloc_copy(TRACEMSG("Null 'thread_id' parameter specified"));
    if (!sl::support::is_uint16_positive(thread_id_len)) return wilton::support::alloc_copy(TRACEMSG(
            "Invalid 'thread_id_len' parameter specified: [" + sl::support::to_string(thread_id_len) + "]"));
    try {
        uint16_t thread_id_len_u16 = static_cast<uint16_t> (thread_id_len);
        auto tid = std::string(thread_id, thread_id_len_u16);
        wilton::internal::clean_duktape_thread_local(tid);
        return nullptr;
    } catch (const std::exception& e) {
        return wilton::support::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

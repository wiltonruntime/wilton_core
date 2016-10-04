/* 
 * File:   wilton_common.cpp
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

namespace { // anonymous

namespace sc = staticlib::config;
namespace su = staticlib::utils;

}

void wilton_free(char* errmsg) {
    std::free(errmsg);
}

WILTON_EXPORT char* wilton_sleep_millis(int millis) {
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

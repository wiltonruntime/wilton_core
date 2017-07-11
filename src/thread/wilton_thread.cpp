/* 
 * File:   wilton_thread.cpp
 * Author: alex
 *
 * Created on December 9, 2016, 10:19 PM
 */

#include "wilton/wilton.h"

#include <cstdint>
#include <thread>
#include <chrono>

#include "staticlib/config.hpp"
#include "staticlib/support.hpp"
#include "staticlib/utils.hpp"

#include "call/wiltoncall_internal.hpp"

namespace { // anonymous

namespace sc = staticlib::config;
namespace su = staticlib::utils;

} // namespace

char* wilton_thread_run(void* cb_ctx, void (*cb)(void* cb_ctx)) /* noexcept */ {
    if (nullptr == cb) return wilton::support::alloc_copy(TRACEMSG("Null 'cb' parameter specified"));
    try {
        auto th = std::thread([cb, cb_ctx]() {
            auto cleaner = sl::support::defer([]() STATICLIB_NOEXCEPT {
                wilton::internal::clean_duktape_thread_local(std::this_thread::get_id());
            });
            try {
                cb(cb_ctx);
            } catch (...) {
                // silently prevent termination
            }
        });
        th.detach();
        return nullptr;
    } catch (const std::exception& e) {
        return wilton::support::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

char* wilton_thread_sleep_millis(int millis) /* noexcept */ {
    if (!sl::support::is_uint32_positive(millis)) return wilton::support::alloc_copy(TRACEMSG(
            "Invalid 'millis' parameter specified: [" + sl::support::to_string(millis) + "]"));
    try {
        uint32_t millis_u32 = static_cast<uint32_t> (millis);
        std::this_thread::sleep_for(std::chrono::milliseconds{millis_u32});
        return nullptr;
    } catch (const std::exception& e) {
        return wilton::support::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

char* wilton_thread_wait_for_signal() {
    try {
        sl::utils::wait_for_signal();
        return nullptr;
    } catch (const std::exception& e) {
        return wilton::support::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

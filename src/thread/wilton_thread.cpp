/* 
 * File:   wilton_thread.cpp
 * Author: alex
 *
 * Created on December 9, 2016, 10:19 PM
 */

#include "wilton/wilton.h"

#include <cstdint>
#include <atomic>
#include <chrono>
#include <thread>

#include "staticlib/config.hpp"
#include "staticlib/support.hpp"
#include "staticlib/utils.hpp"

#include "call/wiltoncall_internal.hpp"
#include "common/wilton_internal_exception.hpp"

namespace { // anonymous

std::atomic<bool>& static_signal_waiter_registered() {
    static std::atomic<bool> flag{false};
    return flag;
}

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

// note: sl::utils::initialize_signals() cannot be called directly from another DLL/EXE
char* wilton_thread_initialize_signals() {
    try {
        sl::utils::initialize_signals();
        return nullptr;
    } catch (const std::exception& e) {
        return wilton::support::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

char* wilton_thread_wait_for_signal() {
    try {
        bool the_false = false;
        bool changed = static_signal_waiter_registered().compare_exchange_strong(the_false, true, std::memory_order_acq_rel);
        if(!changed) {
            throw wilton::common::wilton_internal_exception(TRACEMSG(
                    "Signal waiting thread is already registered"));
        }
        sl::utils::wait_for_signal();
        return nullptr;
    } catch (const std::exception& e) {
        return wilton::support::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

char* wilton_thread_signal_waiters_count(int* count_out) {
    if (nullptr == count_out) return wilton::support::alloc_copy(TRACEMSG("Null 'count_out' parameter specified"));
    try {
        if (static_signal_waiter_registered().load(std::memory_order_acquire)) {
            *count_out = 1;
        } else {
            *count_out = 0;
        }
        return nullptr;
    } catch (const std::exception& e) {
        return wilton::support::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

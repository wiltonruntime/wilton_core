/* 
 * File:   wilton_signal.cpp
 * Author: alex
 *
 * Created on September 22, 2017, 8:03 PM
 */

#include "wilton/wilton.h"

#include <atomic>

#include "staticlib/utils.hpp"

#include "wilton/support/alloc_copy.hpp"
#include "wilton/support/exception.hpp"

namespace { // anonymous

std::atomic<bool>& static_signal_waiter_registered() {
    static std::atomic<bool> flag{false};
    return flag;
}

} // namespace

// note: sl::utils::initialize_signals() cannot be called directly from another DLL/EXE
char* wilton_signal_initialize() {
    try {
        sl::utils::initialize_signals();
        sl::utils::register_signal_listener([] {
            if (!static_signal_waiter_registered().load(std::memory_order_acquire)) {
                std::abort();
            }
        });
        return nullptr;
    } catch (const std::exception& e) {
        return wilton::support::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

char* wilton_signal_await() {
    try {
        bool the_false = false;
        bool changed = static_signal_waiter_registered().compare_exchange_strong(the_false, true, std::memory_order_acq_rel);
        if(!changed) {
            throw wilton::support::exception(TRACEMSG(
                    "Signal waiting thread is already registered"));
        }
        sl::utils::wait_for_signal();
        return nullptr;
    } catch (const std::exception& e) {
        return wilton::support::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

char* wilton_signal_fire() {
    try {
        sl::utils::fire_signal();
        return nullptr;
    } catch (const std::exception& e) {
        return wilton::support::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }

}

/* 
 * File:   wilton_mutex.cpp
 * Author: alex
 *
 * Created on October 4, 2016, 7:37 PM
 */

#include "wilton/wilton.h"

#include "staticlib/utils.hpp"

#include "mutex/Mutex.hpp"

namespace { // anonymous

namespace sc = staticlib::config;
namespace su = staticlib::utils;
namespace wm = wilton::mutex;

}

struct wilton_Mutex {
private:
    wm::Mutex mutex;

public:

    wilton_Mutex(wm::Mutex&& mutex) :
    mutex(std::move(mutex)) { }

    wm::Mutex& impl() {
        return mutex;
    }
};

char* wilton_Mutex_create(wilton_Mutex** mutex_out) /* noexcept */ {
    if (nullptr == mutex_out) return su::alloc_copy(TRACEMSG("Null 'mutex_out' parameter specified"));
    try {
        wm::Mutex mutex{};
        wilton_Mutex* mutex_ptr = new wilton_Mutex{std::move(mutex)};
        *mutex_out = mutex_ptr;
        return nullptr;
    } catch (const std::exception& e) {
        return su::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

char* wilton_Mutex_lock(wilton_Mutex* mutex) /* noexcept */ {
    if (nullptr == mutex) return su::alloc_copy(TRACEMSG("Null 'mutex' parameter specified"));
    try {
        mutex->impl().lock();
        return nullptr;
    } catch (const std::exception& e) {
        return su::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

char* wilton_Mutex_unlock(wilton_Mutex* mutex) /* noexcept */ {
    if (nullptr == mutex) return su::alloc_copy(TRACEMSG("Null 'mutex' parameter specified"));
    try {
        mutex->impl().unlock();
        return nullptr;
    } catch (const std::exception& e) {
        return su::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

char* wilton_Mutex_wait(wilton_Mutex* mutex, int timeout_millis, void* cond_ctx,
        int (*cond_cb)(void* cond_ctx)) /* noexcept */ {
    if (nullptr == mutex) return su::alloc_copy(TRACEMSG("Null 'mutex' parameter specified"));
    if (!sc::is_uint32_positive(timeout_millis)) return su::alloc_copy(TRACEMSG(
            "Invalid 'timeout_millis' parameter specified: [" + sc::to_string(timeout_millis) + "]"));
    if (nullptr == cond_cb) return su::alloc_copy(TRACEMSG("Null 'cond_cb' parameter specified"));
    try {
        uint32_t timeout_millis_u32 = static_cast<uint32_t> (timeout_millis);
        mutex->impl().wait(timeout_millis_u32, [cond_ctx, cond_cb]() {
            return 0 != cond_cb(cond_ctx);
        });
        return nullptr;
    } catch (const std::exception& e) {
        return su::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

char* wilton_Mutex_notify_all(wilton_Mutex* mutex) /* noexcept */ {
    if (nullptr == mutex) return su::alloc_copy(TRACEMSG("Null 'mutex' parameter specified"));
    try {
        mutex->impl().notify_all();
        return nullptr;
    } catch (const std::exception& e) {
        return su::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

// todo: think about destroy sync
char* wilton_Mutex_destroy(wilton_Mutex* mutex) /* noexcept */ {
    if (nullptr == mutex) return su::alloc_copy(TRACEMSG("Null 'mutex' parameter specified"));
    try {
        delete mutex;
        return nullptr;
    } catch (const std::exception& e) {
        return su::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

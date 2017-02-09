/* 
 * File:   waitable_mutex.cpp
 * Author: alex
 * 
 * Created on October 4, 2016, 7:21 PM
 */

#include "mutex/waitable_mutex.hpp"

#include <chrono>
#include <mutex>
#include <condition_variable>

#include "staticlib/config.hpp"
#include "staticlib/pimpl/pimpl_forward_macros.hpp"

namespace wilton {
namespace mutex {

namespace { // anonymous

namespace sc = staticlib::config;

using cond_fun = std::function<bool()>;

} // namespace

class waitable_mutex::impl : public staticlib::pimpl::pimpl_object::impl {
    std::mutex mutex;
    std::condition_variable cv;

public:
    impl() {}
    
    void lock(waitable_mutex&) {
        std::unique_lock<std::mutex> guard{mutex};
        guard.release();
    }

    void unlock(waitable_mutex&) {
        std::unique_lock<std::mutex> guard{mutex, std::adopt_lock};
    }

    void wait(waitable_mutex&, uint32_t timeout_millis, std::function<bool()> cond) {
        std::unique_lock<std::mutex> guard{mutex, std::adopt_lock};
        cv.wait_for(guard, std::chrono::milliseconds(timeout_millis), cond);
        if (!cond()) {
            throw common::wilton_internal_exception(TRACEMSG(
                    "waitable_mutex wait timed out, timeout_millis: [" + sc::to_string(timeout_millis) + "]"));
        }
    }

    void notify_all(waitable_mutex&) {
        cv.notify_all();
    }
    
};

PIMPL_FORWARD_CONSTRUCTOR(waitable_mutex, (), (), common::wilton_internal_exception)
PIMPL_FORWARD_METHOD(waitable_mutex, void, lock, (), (), common::wilton_internal_exception)
PIMPL_FORWARD_METHOD(waitable_mutex, void, unlock, (), (), common::wilton_internal_exception)
PIMPL_FORWARD_METHOD(waitable_mutex, void, wait, (uint32_t)(cond_fun), (), common::wilton_internal_exception)
PIMPL_FORWARD_METHOD(waitable_mutex, void, notify_all, (), (), common::wilton_internal_exception)

} // namespace
}

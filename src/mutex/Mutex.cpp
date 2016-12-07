/* 
 * File:   Mutex.cpp
 * Author: alex
 * 
 * Created on October 4, 2016, 7:21 PM
 */

#include "mutex/Mutex.hpp"

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

class Mutex::Impl : public staticlib::pimpl::PimplObject::Impl {
    std::mutex mutex;
    std::condition_variable cv;

public:
    Impl() {}
    
    void lock(Mutex&) {
        std::unique_lock<std::mutex> guard{mutex};
        guard.release();
    }

    void unlock(Mutex&) {
        std::unique_lock<std::mutex> guard{mutex, std::adopt_lock};
    }

    void wait(Mutex&, uint32_t timeout_millis, std::function<bool()> cond) {
        std::unique_lock<std::mutex> guard{mutex, std::adopt_lock};
        cv.wait_for(guard, std::chrono::milliseconds(timeout_millis), cond);
        if (!cond()) {
            throw common::WiltonInternalException(TRACEMSG(
                    "Mutex wait timed out, timeout_millis: [" + sc::to_string(timeout_millis) + "]"));
        }
    }

    void notify_all(Mutex&) {
        cv.notify_all();
    }
    
};

PIMPL_FORWARD_CONSTRUCTOR(Mutex, (), (), common::WiltonInternalException)
PIMPL_FORWARD_METHOD(Mutex, void, lock, (), (), common::WiltonInternalException)
PIMPL_FORWARD_METHOD(Mutex, void, unlock, (), (), common::WiltonInternalException)
PIMPL_FORWARD_METHOD(Mutex, void, wait, (uint32_t)(cond_fun), (), common::WiltonInternalException)
PIMPL_FORWARD_METHOD(Mutex, void, notify_all, (), (), common::WiltonInternalException)

} // namespace
}

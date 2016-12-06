/* 
 * File:   Mutex.cpp
 * Author: alex
 * 
 * Created on October 4, 2016, 7:21 PM
 */

#include "mutex/Mutex.hpp"

#include <mutex>
#include <condition_variable>

#include "staticlib/config.hpp"
#include "staticlib/pimpl/pimpl_forward_macros.hpp"

namespace wilton {
namespace mutex {

namespace { // anonymous

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

    void wait(Mutex&, std::function<bool()> cond) {
        std::unique_lock<std::mutex> guard{mutex, std::adopt_lock};
        cv.wait(guard, cond);        
    }

    void notify_all(Mutex&) {
        cv.notify_all();
    }
    
};

PIMPL_FORWARD_CONSTRUCTOR(Mutex, (), (), common::WiltonInternalException)
PIMPL_FORWARD_METHOD(Mutex, void, lock, (), (), common::WiltonInternalException)
PIMPL_FORWARD_METHOD(Mutex, void, unlock, (), (), common::WiltonInternalException)
PIMPL_FORWARD_METHOD(Mutex, void, wait, (cond_fun), (), common::WiltonInternalException)
PIMPL_FORWARD_METHOD(Mutex, void, notify_all, (), (), common::WiltonInternalException)

} // namespace
}

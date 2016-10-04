/* 
 * File:   Mutex.cpp
 * Author: alex
 * 
 * Created on October 4, 2016, 7:21 PM
 */

#include "mutex/Mutex.hpp"

#include <memory>
#include <mutex>

#include "staticlib/config.hpp"
#include "staticlib/pimpl/pimpl_forward_macros.hpp"

namespace wilton {
namespace mutex {

class Mutex::Impl : public staticlib::pimpl::PimplObject::Impl {
    std::mutex mutex;
    std::unique_ptr<std::lock_guard<std::mutex>> guard;
    
    std::mutex local_mutex;

public:
    Impl() {}

    void lock(Mutex&) {
        std::lock_guard<std::mutex> local_guard{local_mutex};
        std::lock_guard<std::mutex>* guard_ptr = new std::lock_guard<std::mutex>(mutex);
        this->guard = std::unique_ptr<std::lock_guard<std::mutex>>(guard_ptr);
    }

    void unlock(Mutex&) {
        std::lock_guard<std::mutex> local_guard{local_mutex};
        if (nullptr == guard.get()) throw common::WiltonInternalException(TRACEMSG(
                "Invalid state, mutex is not locked"));
        this->guard.reset();
    }
};

PIMPL_FORWARD_CONSTRUCTOR(Mutex, (), (), common::WiltonInternalException)
PIMPL_FORWARD_METHOD(Mutex, void, lock, (), (), common::WiltonInternalException)
PIMPL_FORWARD_METHOD(Mutex, void, unlock, (), (), common::WiltonInternalException)

} // namespace
}


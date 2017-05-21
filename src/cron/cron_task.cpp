/* 
 * File:   cron_task.cpp
 * Author: alex
 * 
 * Created on September 7, 2016, 12:51 PM
 */

#include "cron/cron_task.hpp"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

#include "staticlib/support.hpp"
#include "staticlib/cron.hpp"
#include "staticlib/pimpl/forward_macros.hpp"

#include "call/wiltoncall_internal.hpp"

namespace wilton {
namespace cron {

namespace { // anonymous

using task_fun_type = std::function<void()>;

} //namespace

class cron_task::impl : public staticlib::pimpl::object::impl {
    std::mutex mutex;
    std::condition_variable cv;
    
    sl::cron::expression cron;
    std::function<void()> task;
    std::thread worker;
    std::atomic<bool> running;
    
public:
    ~impl() STATICLIB_NOEXCEPT {};
    
    impl(const std::string& cronexpr, std::function<void()> crontask) :
    cron(cronexpr),
    task(std::move(crontask)),
    running(true) {
        worker = std::thread([this] {
            auto cleaner = sl::support::defer([]() STATICLIB_NOEXCEPT {
                wilton::duktape::clean_thread_local(std::this_thread::get_id());
            });
            while (running.load()) {
                auto secs = cron.next<std::chrono::seconds>();
                {
                    std::unique_lock<std::mutex> guard{mutex};
                    cv.wait_for(guard, secs, [this]{
                        return !running.load();
                    });
                }
                if (running.load()) {
                    task();
                }
            }
        });
    }
        
    void stop(cron_task&) {
        running.store(false);
        cv.notify_all();
        worker.join();
    }
};
PIMPL_FORWARD_CONSTRUCTOR(cron_task, (const std::string&)(task_fun_type), (), common::wilton_internal_exception)
PIMPL_FORWARD_METHOD(cron_task, void, stop, (), (), common::wilton_internal_exception)

} // namespace
}

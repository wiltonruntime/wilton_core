/* 
 * File:   CronTask.cpp
 * Author: alex
 * 
 * Created on September 7, 2016, 12:51 PM
 */

#include "cron/CronTask.hpp"

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>

#include "staticlib/cron.hpp"
#include "staticlib/pimpl/pimpl_forward_macros.hpp"

namespace wilton {
namespace cron {

namespace { // anonymous

namespace cr = staticlib::cron;

using task_fun_type = std::function<void()>;

} //namespace

class CronTask::Impl : public staticlib::pimpl::PimplObject::Impl {
    std::shared_ptr<std::atomic<bool>> running;
    
public:
    Impl(const std::string& cronexpr, std::function<void()> task) :
    running(new std::atomic<bool>(true)) {
        // check that expr is valid
        auto tmp = cr::CronExpression(cronexpr);
        (void) tmp;
        auto shared_running = this->running;
        auto worker = std::thread([shared_running, cronexpr, task] {
            auto cron = cr::CronExpression(cronexpr);
            while (shared_running->load()) {
                std::chrono::seconds secs = cron.next();
                std::this_thread::sleep_for(secs);
                if (shared_running->load()) {
                    task();
                }
            }
        });
        worker.detach();
    }
        
    void stop(CronTask&) {
        this->running->store(false);
    }
};
PIMPL_FORWARD_CONSTRUCTOR(CronTask, (const std::string&)(task_fun_type), (), common::WiltonInternalException)
PIMPL_FORWARD_METHOD(CronTask, void, stop, (), (), common::WiltonInternalException)

} // namespace
}

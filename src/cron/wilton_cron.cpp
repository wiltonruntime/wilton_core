/* 
 * File:   wilton_cron.cpp
 * Author: alex
 * 
 * Created on September 7, 2016, 8:22 PM
 */

#include "wilton/wilton.h"

#include <string>

#include "staticlib/config.hpp"
#include "staticlib/utils.hpp"

#include "cron/CronTask.hpp"

namespace { // anonymous

namespace sc = staticlib::config;
namespace su = staticlib::utils;
namespace wcr = wilton::cron;

} // namespace

struct wilton_CronTask {
private:
    wcr::CronTask cron;

public:
    wilton_CronTask(wcr::CronTask&& cron) :
    cron(std::move(cron)) { }

    wcr::CronTask& impl() {
        return cron;
    }
};


char* wilton_CronTask_start(
        wilton_CronTask** cron_out,
        const char* cronexpr,
        int cronexpr_len,
        void* task_ctx,
        void (*task_cb)(
                void* task_ctx)) /* noexcept */ {
    if (nullptr == cron_out) return su::alloc_copy(TRACEMSG("Null 'cron_out' parameter specified"));
    if (nullptr == cronexpr) return su::alloc_copy(TRACEMSG("Null 'cronexpr' parameter specified"));
    if (!sc::is_uint16_positive(cronexpr_len)) return su::alloc_copy(TRACEMSG(
            "Invalid 'cronexpr_len' parameter specified: [" + sc::to_string(cronexpr_len) + "]"));
    if (nullptr == task_cb) return su::alloc_copy(TRACEMSG("Null 'task_cb' parameter specified"));
    try {
        uint16_t cronexpr_len_u16 = static_cast<uint16_t> (cronexpr_len);
        std::string cronexpr_str{cronexpr, cronexpr_len_u16};
        wcr::CronTask cron{cronexpr_str, [task_ctx, task_cb]{
            task_cb(task_ctx);
        }};
        wilton_CronTask* cron_ptr = new wilton_CronTask{std::move(cron)};
        *cron_out = cron_ptr;
        return nullptr;
    } catch (const std::exception& e) {
        return su::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

WILTON_EXPORT char* wilton_CronTask_stop(
        wilton_CronTask* cron) {
    if (nullptr == cron) return su::alloc_copy(TRACEMSG("Null 'cron' parameter specified"));
    try {
        cron->impl().stop();
        delete cron;
        return nullptr;
    } catch (const std::exception& e) {
        return su::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

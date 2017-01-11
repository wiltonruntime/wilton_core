/* 
 * File:   wiltoncall_cron.cpp
 * Author: alex
 *
 * Created on January 11, 2017, 9:19 AM
 */

#include "call/wiltoncall_internal.hpp"

#include "wilton/wilton.h"
#include "wilton/wiltoncall.h"

namespace wilton {
namespace cron {

namespace { //anonymous

namespace ss = staticlib::serialization;

common::handle_registry<wilton_CronTask>& static_registry() {
    static common::handle_registry<wilton_CronTask> registry;
    return registry;
}

// shouldn't be called before logging is initialized by app
void log_error(const std::string& message) {
    static std::string level = "ERROR";
    static std::string logger = "wilton.cron";
    // call wilton
    wilton_logger_log(level.c_str(), level.length(), logger.c_str(), logger.length(),
            message.c_str(), message.length());
}

} // namespace

std::string cron_start(const std::string& data) {
    // json parse
    ss::JsonValue json = ss::load_json_from_string(data);
    auto rcallback = std::ref(common::empty_json());
    auto rexpr = std::ref(common::empty_string());
    for (const ss::JsonField& fi : json.as_object()) {
        auto& name = fi.name();
        if ("callback" == name) {
            rcallback = fi.value();
        } else if ("expression" == name) {
            rexpr = common::get_json_string(fi);
        } else {
            throw common::WiltonInternalException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (ss::JsonType::NULL_T == rcallback.get().type()) throw common::WiltonInternalException(TRACEMSG(
            "Required parameter 'callback' not specified"));
    if (rexpr.get().empty()) throw common::WiltonInternalException(TRACEMSG(
            "Required parameter 'url' not specified"));
    const ss::JsonValue& callback = rcallback.get();
    std::string* callback_str_ptr = new std::string();
    *callback_str_ptr = ss::dump_json_to_string(callback);
    const std::string& expr = rexpr.get();
    // call wilton
    wilton_CronTask* cron;
    char* err = wilton_CronTask_start(std::addressof(cron), expr.c_str(), expr.length(),
            callback_str_ptr,
            [](void* passed) {
                std::string* sptr = static_cast<std::string*> (passed);
                // output will be ignored
                char* out;
                int out_len;
                auto err = wiltoncall_runscript(sptr->c_str(), static_cast<int> (sptr->length()),
                        std::addressof(out), std::addressof(out_len));
                delete sptr;
                if (nullptr != err) {
                    log_error(TRACEMSG(err));
                    wilton_free(err);
                }
            });
    if (nullptr != err) common::throw_wilton_error(err, TRACEMSG(err));
    int64_t handle = static_registry().put(cron);
    return ss::dump_json_to_string({
        { "cronHandle", handle}
    });
}

std::string cron_stop(const std::string& data) {
    // json parse
    ss::JsonValue json = ss::load_json_from_string(data);
    int64_t handle = -1;
    for (const ss::JsonField& fi : json.as_object()) {
        auto& name = fi.name();
        if ("cronHandle" == name) {
            handle = common::get_json_int64(fi);
        } else {
            throw common::WiltonInternalException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw common::WiltonInternalException(TRACEMSG(
            "Required parameter 'httpclientHandle' not specified"));
    // get handle
    wilton_CronTask* cron = static_registry().remove(handle);
    if (nullptr == cron) throw common::WiltonInternalException(TRACEMSG(
            "Invalid 'cronHandle' parameter specified"));
    // call wilton
    char* err = wilton_CronTask_stop(cron);
    if (nullptr != err) {
        static_registry().put(cron);
        common::throw_wilton_error(err, TRACEMSG(err));
    }
    return "{}";
}

} // namespace
}

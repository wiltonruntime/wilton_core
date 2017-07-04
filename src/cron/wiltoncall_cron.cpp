/* 
 * File:   wiltoncall_cron.cpp
 * Author: alex
 *
 * Created on January 11, 2017, 9:19 AM
 */

#include "call/wiltoncall_internal.hpp"

#include <atomic>
#include <memory>

#include "wilton/wilton.h"
#include "wilton/wiltoncall.h"

#include "logging/logging_internal.hpp"

namespace wilton {
namespace cron {

namespace { //anonymous

support::payload_handle_registry<wilton_CronTask, std::unique_ptr<std::string>>& static_registry() {
    static support::payload_handle_registry<wilton_CronTask, std::unique_ptr<std::string>> registry{
        [] (wilton_CronTask* cron) STATICLIB_NOEXCEPT {
            wilton_CronTask_stop(cron);
        }};
    return registry;
}

} // namespace

sl::support::optional<sl::io::span<char>> cron_start(sl::io::span<const char> data) {
    // json parse
    auto json = sl::json::load(data);
    auto rcallback = std::ref(sl::json::null_value_ref());
    auto rexpr = std::ref(sl::utils::empty_string());
    for (const sl::json::field& fi : json.as_object()) {
        auto& name = fi.name();
        if ("callbackScript" == name) {
            common::check_json_callback_script(fi);
            rcallback = fi.val();
        } else if ("expression" == name) {
            rexpr = fi.as_string_nonempty_or_throw(name);
        } else {
            throw common::wilton_internal_exception(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (sl::json::type::nullt == rcallback.get().json_type()) throw common::wilton_internal_exception(TRACEMSG(
            "Required parameter 'callbackScript' not specified"));
    if (rexpr.get().empty()) throw common::wilton_internal_exception(TRACEMSG(
            "Required parameter 'url' not specified"));
    const sl::json::value& callback = rcallback.get();
    const std::string& expr = rexpr.get();
    std::string* str_to_pass = new std::string(callback.dumps());
    // call wilton
    wilton_CronTask* cron;
    char* err = wilton_CronTask_start(std::addressof(cron), expr.c_str(), static_cast<int>(expr.length()),
            static_cast<void*> (str_to_pass),
            [](void* passed) {
                std::string* str = static_cast<std::string*> (passed);
                sl::json::value cb_json = sl::json::loads(*str);
                std::string engine = cb_json["engine"].as_string();
                // output will be ignored
                char* out = nullptr;
                int out_len = 0;
                auto err = wiltoncall_runscript(engine.c_str(), static_cast<int> (engine.length()), 
                        str->c_str(), static_cast<int> (str->length()),
                        std::addressof(out), std::addressof(out_len));
                if (nullptr != err) {
                    log_error("wilton.cron", TRACEMSG(err));
                    wilton_free(err);
                }
            });
    if (nullptr != err) common::throw_wilton_error(err, TRACEMSG(err));
    int64_t handle = static_registry().put(cron, std::unique_ptr<std::string>(str_to_pass));
    return support::json_span({
        { "cronHandle", handle}
    });
}

sl::support::optional<sl::io::span<char>> cron_stop(sl::io::span<const char> data) {
    // json parse
    auto json = sl::json::load(data);
    int64_t handle = -1;
    for (const sl::json::field& fi : json.as_object()) {
        auto& name = fi.name();
        if ("cronHandle" == name) {
            handle = fi.as_int64_or_throw(name);
        } else {
            throw common::wilton_internal_exception(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw common::wilton_internal_exception(TRACEMSG(
            "Required parameter 'cronHandle' not specified"));
    // get handle
    auto pa = static_registry().remove(handle);
    if (nullptr == pa.first) throw common::wilton_internal_exception(TRACEMSG(
            "Invalid 'cronHandle' parameter specified"));
    // call wilton
    char* err = wilton_CronTask_stop(pa.first);
    if (nullptr != err) {
        static_registry().put(pa.first, std::move(pa.second));
        common::throw_wilton_error(err, TRACEMSG(err));
    }
    return support::empty_span();
}

} // namespace
}

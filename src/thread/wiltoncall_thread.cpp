/* 
 * File:   wiltoncall_thread.cpp
 * Author: alex
 *
 * Created on January 11, 2017, 8:43 AM
 */

#include "call/wiltoncall_internal.hpp"

#include "wilton/wilton.h"
#include "wilton/wiltoncall.h"

#include "logging/logging_internal.hpp"

namespace wilton {
namespace thread {

support::buffer thread_run(sl::io::span<const char> data) {
    // json parse
    auto json = sl::json::load(data);
    auto rcallback = std::ref(sl::json::null_value_ref());
    for (const sl::json::field& fi : json.as_object()) {
        auto& name = fi.name();
        if ("callbackScript" == name) {
            common::check_json_callback_script(fi);
            rcallback = fi.val();
        } else {
            throw common::wilton_internal_exception(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (sl::json::type::nullt == rcallback.get().json_type()) throw common::wilton_internal_exception(TRACEMSG(
            "Required parameter 'callbackScript' not specified"));
    const sl::json::value& callback = rcallback.get();
    std::string* callback_str_ptr = new std::string();
    *callback_str_ptr = callback.dumps();
    // call wilton
    char* err = wilton_thread_run(callback_str_ptr,
            [](void* passed) {
                std::string* sptr = static_cast<std::string*>(passed);
                sl::json::value cb_json = sl::json::loads(*sptr);
                std::string engine = cb_json["engine"].as_string();
                // output will be ignored
                char* out = nullptr;
                int out_len = 0;
                auto err = wiltoncall_runscript(engine.c_str(), static_cast<int> (engine.length()),
                        sptr->c_str(), static_cast<int>(sptr->length()),
                        std::addressof(out), std::addressof(out_len));
                delete sptr;
                if (nullptr != err) {
                    log_error("wilton.thread", TRACEMSG(err));
                    wilton_free(err);
                }
                if (nullptr != out) {
                    wilton_free(out);
                }
            });
    if (nullptr != err) {
        common::throw_wilton_error(err, TRACEMSG(err));
    }
    return support::make_empty_buffer();
}

support::buffer thread_sleep_millis(sl::io::span<const char> data) {
    // json parse
    auto json = sl::json::load(data);
    int64_t millis = -1;
    for (const sl::json::field& fi : json.as_object()) {
        auto& name = fi.name();
        if ("millis" == name) {
            millis = fi.as_int64_or_throw(name);
        } else {
            throw common::wilton_internal_exception(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == millis) throw common::wilton_internal_exception(TRACEMSG(
            "Required parameter 'millis' not specified"));
    // call wilton
    char* err = wilton_thread_sleep_millis(static_cast<int> (millis));
    if (nullptr != err) {
        common::throw_wilton_error(err, TRACEMSG(err));
    }
    return support::make_empty_buffer();
}

support::buffer thread_wait_for_signal(sl::io::span<const char>) {
    // call wilton
    char* err = wilton_thread_wait_for_signal();
    if (nullptr != err) {
        common::throw_wilton_error(err, TRACEMSG(err));
    }
    return support::make_empty_buffer();
}

support::buffer thread_fire_signal(sl::io::span<const char>) {
    // call wilton
    char* err = wilton_thread_fire_signal();
    if (nullptr != err) {
        common::throw_wilton_error(err, TRACEMSG(err));
    }
    return support::make_empty_buffer();
}


} // namespace
}

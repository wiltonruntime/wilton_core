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

namespace { //anonymous

namespace ss = staticlib::serialization;

} // namespace

std::string thread_run(const std::string& data) {
    // json parse
    ss::JsonValue json = ss::load_json_from_string(data);
    auto rcallback = std::ref(common::empty_json());
    for (const ss::JsonField& fi : json.as_object()) {
        auto& name = fi.name();
        if ("callbackScript" == name) {
            common::check_json_callback_script(fi);
            rcallback = fi.value();
        } else {
            throw common::WiltonInternalException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (ss::JsonType::NULL_T == rcallback.get().type()) throw common::WiltonInternalException(TRACEMSG(
            "Required parameter 'callbackScript' not specified"));
    const ss::JsonValue& callback = rcallback.get();
    std::string* callback_str_ptr = new std::string();
    *callback_str_ptr = ss::dump_json_to_string(callback);
    // call wilton
    char* err = wilton_thread_run(callback_str_ptr,
            [](void* passed) {
                std::string* sptr = static_cast<std::string*>(passed);
                // output will be ignored
                char* out;
                int out_len;
                auto err = wiltoncall_runscript(sptr->c_str(), static_cast<int>(sptr->length()),
                        std::addressof(out), std::addressof(out_len));
                delete sptr;
                if (nullptr != err) {
                    log_error("wilton.thread", TRACEMSG(err));
                    wilton_free(err);
                }
            });
    if (nullptr != err) {
        common::throw_wilton_error(err, TRACEMSG(err));
    }
    return "{}";
}

std::string thread_sleep_millis(const std::string& data) {
    // json parse
    ss::JsonValue json = ss::load_json_from_string(data);
    int64_t millis = -1;
    for (const ss::JsonField& fi : json.as_object()) {
        auto& name = fi.name();
        if ("millis" == name) {
            millis = common::get_json_int64(fi);
        } else {
            throw common::WiltonInternalException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == millis) throw common::WiltonInternalException(TRACEMSG(
            "Required parameter 'millis' not specified"));
    // call wilton
    char* err = wilton_thread_sleep_millis(static_cast<int> (millis));
    if (nullptr != err) {
        common::throw_wilton_error(err, TRACEMSG(err));
    }
    return "{}";
}


} // namespace
}

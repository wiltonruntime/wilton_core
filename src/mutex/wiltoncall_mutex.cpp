/* 
 * File:   wiltoncall_mutex.cpp
 * Author: alex
 *
 * Created on January 11, 2017, 9:34 AM
 */

#include "call/wiltoncall_internal.hpp"

#include "staticlib/config.hpp"
#include "staticlib/io.hpp"
#include "staticlib/utils.hpp"

#include "wilton/wilton.h"
#include "wilton/wiltoncall.h"

#include "logging/logging_internal.hpp"

namespace wilton {
namespace mutex {

namespace { //anonymous

common::handle_registry<wilton_Mutex>& static_registry() {
    static common::handle_registry<wilton_Mutex> registry;
    return registry;
}

} // namespace

std::string mutex_create(const std::string&) {
    wilton_Mutex* mutex;
    char* err = wilton_Mutex_create(std::addressof(mutex));
    if (nullptr != err) common::throw_wilton_error(err, TRACEMSG(std::string(err) +
            "\nmutex_create error"));
    int64_t handle = static_registry().put(mutex);
    return sl::json::dumps({
        { "mutexHandle", handle}
    });
}

std::string mutex_lock(const std::string& data) {
    // json parse
    sl::json::value json = sl::json::loads(data);
    int64_t handle = -1;
    for (const sl::json::field& fi : json.as_object()) {
        auto& name = fi.name();
        if ("mutexHandle" == name) {
            handle = fi.as_int64_or_throw(name);
        } else {
            throw common::wilton_internal_exception(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw common::wilton_internal_exception(TRACEMSG(
            "Required parameter 'mutexHandle' not specified"));
    // get handle
    wilton_Mutex* mutex = static_registry().peek(handle);
    if (nullptr == mutex) throw common::wilton_internal_exception(TRACEMSG(
            "Invalid 'mutexHandle' parameter specified"));
    // call wilton
    char* err = wilton_Mutex_lock(mutex);
    if (nullptr != err) {
        common::throw_wilton_error(err, TRACEMSG(err));
    }
    return "{}";
}

std::string mutex_unlock(const std::string& data) {
    // json parse
    sl::json::value json = sl::json::loads(data);
    int64_t handle = -1;
    for (const sl::json::field& fi : json.as_object()) {
        auto& name = fi.name();
        if ("mutexHandle" == name) {
            handle = fi.as_int64_or_throw(name);
        } else {
            throw common::wilton_internal_exception(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw common::wilton_internal_exception(TRACEMSG(
            "Required parameter 'mutexHandle' not specified"));
    // get handle
    wilton_Mutex* mutex = static_registry().peek(handle);
    if (nullptr == mutex) throw common::wilton_internal_exception(TRACEMSG(
            "Invalid 'mutexHandle' parameter specified"));
    // call wilton
    char* err = wilton_Mutex_unlock(mutex);
    if (nullptr != err) {
        common::throw_wilton_error(err, TRACEMSG(err));
    }
    return "{}";
}

std::string mutex_wait(const std::string& data) {
    // json parse
    sl::json::value json = sl::json::loads(data);
    auto rcallback = std::ref(sl::json::null_value_ref());
    int64_t handle = -1;
    int64_t timeout_millis = -1;
    for (const sl::json::field& fi : json.as_object()) {
        auto& name = fi.name();
        if ("conditionCallbackScript" == name) {
            common::check_json_callback_script(fi);
            rcallback = fi.val();
        } else if ("mutexHandle" == name) {
            handle = fi.as_int64_or_throw(name);
        } else if ("timeoutMillis" == name) {
            timeout_millis = fi.as_int64_or_throw(name);
        } else {
            throw common::wilton_internal_exception(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (sl::json::type::nullt == rcallback.get().json_type()) throw common::wilton_internal_exception(TRACEMSG(
            "Required parameter 'conditionCallbackScript' not specified"));
    if (-1 == handle) throw common::wilton_internal_exception(TRACEMSG(
            "Required parameter 'mutexHandle' not specified"));
    if (-1 == timeout_millis) throw common::wilton_internal_exception(TRACEMSG(
            "Required parameter 'timeoutMillis' not specified"));
    const sl::json::value& callback = rcallback.get();
    std::string cbjson = callback.dumps();
    // get handle
    wilton_Mutex* mutex = static_registry().peek(handle);
    if (nullptr == mutex) throw common::wilton_internal_exception(TRACEMSG(
            "Invalid 'mutexHandle' parameter specified"));
    // call wilton
    char* err = wilton_Mutex_wait(mutex, static_cast<int> (timeout_millis), static_cast<void*> (std::addressof(cbjson)),
            [](void* passed) {
                std::string* sptr = static_cast<std::string*> (passed);
                char* out;
                int out_len;
                auto err = wiltoncall_runscript(sptr->c_str(), static_cast<int> (sptr->length()),
                        std::addressof(out), std::addressof(out_len));                
                if (nullptr != err) {
                    log_error("wilton.mutex", TRACEMSG(err));
                    wilton_free(err);
                    return 1;
                }
                if (nullptr == out) {
                    log_error("wilton.mutex", TRACEMSG("Null condition result returned"));
                    return 1;
                }
                if (!sl::support::is_uint16_positive(out_len)) {
                    log_error("wilton.mutex", TRACEMSG(
                        "Invalid output length returned from condition: [" + sl::support::to_string(out_len) + "]"));
                    return 1;
                }
                // parse json
                auto src = sl::io::array_source(out, static_cast<uint16_t> (out_len));
                sl::json::value res = sl::json::load(src);
                int32_t tribool = -1;
                for (const sl::json::field& fi : res.as_object()) {
                    auto& name = fi.name();
                    if ("condition" == name && sl::json::type::boolean == fi.json_type()) {
                        tribool = fi.as_bool() ? 1 : 0;
                    } else {
                        log_error("wilton.mutex", TRACEMSG("Unknown condition result data field: [" + name + "]"));
                        return 1;
                    }
                }
                if (-1 == tribool) {
                    std::string res_str{out, static_cast<uint16_t> (out_len)};
                    log_error("wilton.mutex", TRACEMSG("Required parameter 'condition' not specified" +
                            " in condition result: [" + res_str + "]"));
                }
                bool cond_res = 1 == tribool;
                return cond_res ? 1 : 0;
            });
    if (nullptr != err) {
        common::throw_wilton_error(err, TRACEMSG(err));
    }
    return "{}";
}

std::string mutex_notify_all(const std::string& data) {
    // json parse
    sl::json::value json = sl::json::loads(data);
    int64_t handle = -1;
    for (const sl::json::field& fi : json.as_object()) {
        auto& name = fi.name();
        if ("mutexHandle" == name) {
            handle = fi.as_int64_or_throw(name);
        } else {
            throw common::wilton_internal_exception(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw common::wilton_internal_exception(TRACEMSG(
            "Required parameter 'mutexHandle' not specified"));
    // get handle
    wilton_Mutex* mutex = static_registry().peek(handle);
    if (nullptr == mutex) throw common::wilton_internal_exception(TRACEMSG(
            "Invalid 'mutexHandle' parameter specified"));
    // call wilton
    char* err = wilton_Mutex_notify_all(mutex);
    if (nullptr != err) {
        common::throw_wilton_error(err, TRACEMSG(err));
    }
    return "{}";
}

std::string mutex_destroy(const std::string& data) {
    // json parse
    sl::json::value json = sl::json::loads(data);
    int64_t handle = -1;
    for (const sl::json::field& fi : json.as_object()) {
        auto& name = fi.name();
        if ("mutexHandle" == name) {
            handle = fi.as_int64_or_throw(name);
        } else {
            throw common::wilton_internal_exception(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw common::wilton_internal_exception(TRACEMSG(
            "Required parameter 'mutexHandle' not specified"));
    // get handle
    wilton_Mutex* mutex = static_registry().remove(handle);
    if (nullptr == mutex) throw common::wilton_internal_exception(TRACEMSG(
            "Invalid 'mutexHandle' parameter specified"));
    // call wilton
    char* err = wilton_Mutex_destroy(mutex);
    if (nullptr != err) {
        static_registry().put(mutex);
        common::throw_wilton_error(err, TRACEMSG(err));
    }
    return "{}";
}


} // namespace
}

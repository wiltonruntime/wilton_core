/*
 * Copyright 2017, alex at staticlibs.net
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* 
 * File:   wiltoncall.cpp
 * Author: alex
 *
 * Created on January 8, 2017, 1:44 PM
 */

#include "wilton/wiltoncall.h"

#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <utility>
#include <unordered_set>

#include "staticlib/config.hpp"
#include "staticlib/tinydir.hpp"
#include "staticlib/utils.hpp"

#include "wilton/support/exception.hpp"
#include "wilton/support/misc.hpp"
#include "wilton/support/registrar.hpp"

#include "call/wiltoncall_internal.hpp"
#include "call/wiltoncall_registry.hpp"

namespace { // anonymous

std::atomic_flag initialized = ATOMIC_FLAG_INIT;
const size_t max_registry_entries_count = 1 << 16;

// initialized from wiltoncall_init
std::shared_ptr<wilton::internal::wiltoncall_registry> shared_registry() {
    static auto reg = std::make_shared<wilton::internal::wiltoncall_registry>(max_registry_entries_count);
    return reg;
}

} // namespace

namespace wilton {
namespace internal {

// initialized from wiltoncall_init
std::shared_ptr<sl::json::value> shared_wiltoncall_config(const std::string& cf_json) {
    static auto cf = std::make_shared<sl::json::value>(sl::json::loads(cf_json));
    return cf;
}

} // namespace
}

char* wiltoncall_init(const char* config_json, int config_json_len) {
    if (nullptr == config_json) return wilton::support::alloc_copy(TRACEMSG("Null 'default_script_engine_name' parameter specified"));
    if (!sl::support::is_uint16_positive(config_json_len)) return wilton::support::alloc_copy(TRACEMSG(
            "Invalid 'config_json_len' parameter specified: [" + sl::support::to_string(config_json_len) + "]"));

    try {
        // check called once
        if (initialized.test_and_set(std::memory_order_acq_rel)) {
            throw wilton::support::exception(TRACEMSG("'wiltoncall' registry is already initialized"));
        }
        // set static config
        auto config_json_str = std::string(config_json, static_cast<uint16_t> (config_json_len));
        wilton::internal::shared_wiltoncall_config(config_json_str);

        // init tls cleaners
        wilton::internal::init_tls_cleaners_registry();

        // dyload
        wilton::support::register_wiltoncall("dyload_shared_library", wilton::dyload::dyload_shared_library);
        // misc
        wilton::support::register_wiltoncall("get_wiltoncall_config", wilton::misc::get_wiltoncall_config);
        wilton::support::register_wiltoncall("stdin_readline", wilton::misc::stdin_readline);
        wilton::support::register_wiltoncall("wiltoncall_list_registered", wilton::misc::wiltoncall_list_registered);

        return nullptr;
    } catch (const std::exception& e) {
        return wilton::support::alloc_copy(TRACEMSG(e.what() +
                "\n'wiltoncall' initialization error"));
    }
}

char* wiltoncall(const char* call_name, int call_name_len, const char* json_in, int json_in_len,
        char** json_out, int* json_out_len) /* noexcept */ {
    if (nullptr == call_name) return wilton::support::alloc_copy(TRACEMSG("Null 'call_name' parameter specified"));
    if (!sl::support::is_uint16_positive(call_name_len)) return wilton::support::alloc_copy(TRACEMSG(
            "Invalid 'call_name_len' parameter specified: [" + sl::support::to_string(call_name_len) + "]"));
    if (nullptr == json_in) return wilton::support::alloc_copy(TRACEMSG("Null 'json_in' parameter specified"));
    if (!sl::support::is_uint32_positive(json_in_len)) return wilton::support::alloc_copy(TRACEMSG(
            "Invalid 'json_in_len' parameter specified: [" + sl::support::to_string(json_in_len) + "]"));
    if (nullptr == json_out) return wilton::support::alloc_copy(TRACEMSG("Null 'json_out' parameter specified"));
    if (nullptr == json_out_len) return wilton::support::alloc_copy(TRACEMSG("Null 'json_out_len' parameter specified"));
    auto call_name_str = std::string();
    try {
        uint16_t call_name_len_u16 = static_cast<uint16_t> (call_name_len);
        call_name_str = std::string(call_name, call_name_len_u16);
        // get entry
        auto reg = shared_registry();
        auto en = reg->get(call_name_str);
        // invoke function
        wilton::internal::cb_ctx_type cb_ctx = en.first;
        wilton::internal::cb_fun_type cb_fun = en.second;
        char* out = nullptr;
        int out_len = 0;
        auto err = cb_fun(cb_ctx, json_in, json_in_len, std::addressof(out), std::addressof(out_len));
        // check error
        if (nullptr != err) {
            wilton::support::throw_wilton_error(err, TRACEMSG(err));
        }
        // check result
        if (nullptr != out) {
            if (!sl::support::is_uint32(out_len)) {
                throw wilton::support::exception(TRACEMSG(
                        "Invalid result length value returned: [" + sl::support::to_string(out_len) + "]"));
            }
            *json_out = out;
            *json_out_len = out_len;
        } else {
            *json_out = nullptr;
            *json_out_len = 0;
        }
        return nullptr;
    } catch (const std::exception& e) {
        auto data_log = std::string();
        if (json_in_len <= 1024) {
            data_log = std::string(json_in, static_cast<uint32_t> (json_in_len));
        } else {
            data_log.append("length: ");
            data_log.append(sl::support::to_string(json_in_len));
        }
        return wilton::support::alloc_copy(TRACEMSG(e.what() + 
                "\n'wiltoncall' error for name: [" + call_name_str + "]," +
                " data: [" + data_log + "]"));
    }
}

char* wiltoncall_register(const char* call_name, int call_name_len, void* call_ctx,
        char* (*call_cb)
        (void* call_ctx, const char* json_in, int json_in_len, char** json_out, int* json_out_len)) /* noexcept */ {
    if (nullptr == call_name) return wilton::support::alloc_copy(TRACEMSG("Null 'call_name' parameter specified"));
    if (!sl::support::is_uint16_positive(call_name_len)) return wilton::support::alloc_copy(TRACEMSG(
            "Invalid 'call_name_len' parameter specified: [" + sl::support::to_string(call_name_len) + "]"));
    if (nullptr == call_cb) return wilton::support::alloc_copy(TRACEMSG("Null 'call_cb' parameter specified"));
    try {
        auto call_name_str = std::string(call_name, static_cast<uint16_t> (call_name_len));
        auto reg = shared_registry();
        reg->put(call_name_str, call_ctx, call_cb);
        return nullptr;
    } catch (const std::exception& e) {
        return wilton::support::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

char* wiltoncall_list(char** json_list_out, int* json_list_out_len) {
    if (nullptr == json_list_out) return wilton::support::alloc_copy(TRACEMSG("Null 'json_list_out' parameter specified"));
    if (nullptr == json_list_out_len) return wilton::support::alloc_copy(TRACEMSG("Null 'json_list_out_len' parameter specified"));
    try {
        auto reg = shared_registry();
        auto names = reg->list();
        auto list = std::vector<sl::json::value>();
        for (auto&& nm : names) {
            list.emplace_back(sl::json::value(std::move(nm)));
        }
        auto val = sl::json::value(std::move(list));
        auto buf = wilton::support::make_json_buffer(val);
        *json_list_out = buf.data();
        *json_list_out_len = buf.size_int();
        return nullptr;
    } catch (const std::exception& e) {
        return wilton::support::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

char* wiltoncall_remove(const char* call_name, int call_name_len) {
    if (nullptr == call_name) return wilton::support::alloc_copy(TRACEMSG("Null 'call_name' parameter specified"));
    if (!sl::support::is_uint16_positive(call_name_len)) return wilton::support::alloc_copy(TRACEMSG(
            "Invalid 'call_name_len' parameter specified: [" + sl::support::to_string(call_name_len) + "]"));
    try {
        auto call_name_str = std::string(call_name, static_cast<uint16_t> (call_name_len));
        auto reg = shared_registry();
        reg->remove(call_name_str);
        return nullptr;
    } catch (const std::exception& e) {
        return wilton::support::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

#ifndef WILTON_DISABLE_DEFAULT_RUNSCRIPT

char* wiltoncall_runscript(const char* script_engine_name, int script_engine_name_len,
        const char* json_in, int json_in_len, char** json_out, int* json_out_len) {
    // initialized from launcher
    static std::string default_engine = wilton::internal::shared_wiltoncall_config()->getattr("defaultScriptEngine")
            .as_string_nonempty_or_throw("defaultScriptEngine");
    if (nullptr == script_engine_name) return wilton::support::alloc_copy(TRACEMSG("Null 'script_engine_name' parameter specified"));
    if (!sl::support::is_uint16(script_engine_name_len)) return wilton::support::alloc_copy(TRACEMSG(
            "Invalid 'script_engine_name_len' parameter specified: [" + sl::support::to_string(script_engine_name_len) + "]"));
    //if (nullptr == json_in) return wilton::support::alloc_copy(TRACEMSG("Null 'json_in' parameter specified"));
    //if (!sl::support::is_uint32_positive(json_in_len)) return wilton::support::alloc_copy(TRACEMSG(
    //        "Invalid 'json_in_len' parameter specified: [" + sl::support::to_string(json_in_len) + "]"));
    if (nullptr == json_out) return wilton::support::alloc_copy(TRACEMSG("Null 'json_out' parameter specified"));
    if (nullptr == json_out_len) return wilton::support::alloc_copy(TRACEMSG("Null 'json_out_len' parameter specified"));
    try {
        auto engine = std::ref(sl::utils::empty_string());
        auto specified_engine = std::string(script_engine_name, static_cast<uint16_t> (script_engine_name_len));
        engine = !specified_engine.empty() ? specified_engine : default_engine;
        auto callname = "runscript_" + engine.get();
        
        // call engine
        auto err = wiltoncall(callname.c_str(), static_cast<int>(callname.length()),
                json_in, json_in_len, json_out, json_out_len);
        if (nullptr != err) {
            wilton::support::throw_wilton_error(err, TRACEMSG(err));
        }

        return nullptr;
    } catch (const std::exception& e) {
        return wilton::support::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

#endif // WILTON_DISABLE_DEFAULT_RUNSCRIPT

char* wilton_set_thread_capabilities(const char* capabilities_json, int capabilities_json_len) {
    if (nullptr != capabilities_json && !sl::support::is_uint16_positive(capabilities_json_len)) {
            return wilton::support::alloc_copy(TRACEMSG(
                    "Invalid 'capabilities_json_len' parameter specified: [" + sl::support::to_string(capabilities_json_len) + "]"));
    }
    try {
        auto span = sl::io::make_span(capabilities_json, capabilities_json_len);
        auto json = sl::json::load(span);
        auto& vec = json.as_array_or_throw("capabilities");
        if (vec.empty()) throw wilton::support::exception(TRACEMSG(
                "Invalid empty capabilities list specified"));
        auto caps = std::unordered_set<std::string>();
        for (auto& el : vec) {
            auto st = el.as_string_nonempty_or_throw("empty capability");
            auto it = caps.emplace(std::move(st));
            if (!it.second) throw wilton::support::exception(TRACEMSG(
                    "Invalid duplicate capability specified, name: [" + st + "]"));
        }
        auto reg = shared_registry();
        reg->set_thread_caps(std::move(caps));
        return nullptr;
    } catch (const std::exception& e) {
        return wilton::support::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

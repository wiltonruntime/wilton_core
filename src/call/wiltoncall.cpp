/* 
 * File:   wiltoncall.cpp
 * Author: alex
 *
 * Created on January 8, 2017, 1:44 PM
 */

#include "wilton/wiltoncall.h"

#include <atomic>
#include <memory>

#include "staticlib/config.hpp"
#include "staticlib/tinydir.hpp"
#include "staticlib/utils.hpp"

#include "wilton/support/exception.hpp"
#include "wilton/support/misc.hpp"

#include "call/wiltoncall_registry.hpp"
#include "call/wiltoncall_internal.hpp"

namespace { // anonymous

std::shared_ptr<wilton::call::wiltoncall_registry> shared_registry() {
    static auto registry = std::make_shared<wilton::call::wiltoncall_registry>();
    return registry;
}

} // namespace

namespace wilton {
namespace internal {

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
        bool the_false = false;
        static std::atomic<bool> initilized{false};
        if (!initilized.compare_exchange_strong(the_false, true)) {
            throw wilton::support::exception(TRACEMSG("'wiltoncall' registry is already initialized"));
        }
        // set static config
        auto config_json_str = std::string(config_json, static_cast<uint16_t> (config_json_len));
        wilton::internal::shared_wiltoncall_config(config_json_str);

        // registry
        auto reg = shared_registry();

        // dyload
        reg->put("dyload_shared_library", wilton::dyload::dyload_shared_library);
        // misc
        reg->put("get_wiltoncall_config", wilton::misc::get_wiltoncall_config);
        reg->put("stdin_readline", wilton::misc::stdin_readline);

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
    std::string call_name_str = "";
    uint32_t json_in_len_u32 = static_cast<uint32_t> (json_in_len);
    try {
        uint16_t call_name_len_u16 = static_cast<uint16_t> (call_name_len);
        call_name_str = std::string(call_name, call_name_len_u16);
        auto reg = shared_registry();
        auto out = reg->invoke(call_name_str, {json_in, json_in_len_u32});
        if (out) {
            *json_out = out.value().data();
            *json_out_len = static_cast<int>(out.value().size());
        } else {
            *json_out = nullptr;
            *json_out_len = 0;
        }
        return nullptr;
    } catch (const std::exception& e) {
        return wilton::support::alloc_copy(TRACEMSG(e.what() + 
                "\n'wiltoncall' error for name: [" + call_name_str + "]," +
                " data: [" + std::string(json_in, json_in_len_u32) + "]"));
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
        uint16_t call_name_len_u16 = static_cast<uint16_t> (call_name_len);
        std::string call_name_str{call_name, call_name_len_u16};
        auto fun = [call_ctx, call_cb](sl::io::span<const char> data) -> wilton::support::buffer {
            char* out = nullptr;
            int out_len = 0;
            auto err = call_cb(call_ctx, data.data(), static_cast<int>(data.size()), 
                    std::addressof(out), std::addressof(out_len));
            if (nullptr != err) {
                std::string msg = TRACEMSG(std::string(err));
                wilton_free(err);
                throw wilton::support::exception(msg);
            }
            if (nullptr != out) {
                if (!sl::support::is_uint32(out_len)) {
                    throw wilton::support::exception(TRACEMSG(
                            "Invalid result length value returned: [" + sl::support::to_string(out_len) + "]"));
                }
                return wilton::support::wrap_wilton_buffer(out, out_len);
            }
            return wilton::support::make_empty_buffer();
        };
        auto reg = shared_registry();
        reg->put(call_name_str, fun);
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
        uint16_t call_name_len_u16 = static_cast<uint16_t> (call_name_len);
        std::string call_name_str{call_name, call_name_len_u16};
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

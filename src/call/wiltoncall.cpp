/* 
 * File:   wiltoncall.cpp
 * Author: alex
 *
 * Created on January 8, 2017, 1:44 PM
 */

#include "wilton/wiltoncall.h"

#include <atomic>

#include "staticlib/config.hpp"
#include "staticlib/tinydir.hpp"
#include "staticlib/utils.hpp"

#include "wilton/support/exception.hpp"

#include "call/wiltoncall_registry.hpp"
#include "call/wiltoncall_internal.hpp"

namespace { // anonymous

wilton::call::wiltoncall_registry& static_registry() {
    static wilton::call::wiltoncall_registry registry;
    return registry;
}

} // namespace

namespace wilton {
namespace internal {

const sl::json::value& static_wiltoncall_config(const std::string& cf_json) {
    static sl::json::value cf = sl::json::loads(cf_json);
    return cf;
}

sl::support::observer_ptr<sl::unzip::file_index> static_modules_idx(sl::unzip::file_index* index) {
    static std::unique_ptr<sl::unzip::file_index> idx = std::unique_ptr<sl::unzip::file_index>(index);
    return sl::support::make_observer_ptr(idx.get());
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
        wilton::internal::static_wiltoncall_config(config_json_str);
                
        // init static modules index
        auto cf = sl::json::loads(config_json_str);
        auto modpath = cf["requireJs"]["baseUrl"].as_string_nonempty_or_throw("requireJs.baseUrl");
        if (sl::utils::starts_with(modpath, wilton::internal::zip_proto_prefix)) {
            auto zippath = modpath.substr(wilton::internal::zip_proto_prefix.length());
            auto zippath_norm = sl::tinydir::normalize_path(zippath);
            wilton::internal::static_modules_idx(new sl::unzip::file_index(zippath_norm));
        }
        
        // registry
        auto& reg = static_registry();
        
        // server
        reg.put("server_create", wilton::server::server_create);
        reg.put("server_stop", wilton::server::server_stop);
        reg.put("request_get_metadata", wilton::server::request_get_metadata);
        reg.put("request_get_data", wilton::server::request_get_data);
        reg.put("request_get_form_data", wilton::server::request_get_form_data);
        reg.put("request_get_data_filename", wilton::server::request_get_data_filename);
        reg.put("request_set_response_metadata", wilton::server::request_set_response_metadata);
        reg.put("request_send_response", wilton::server::request_send_response);
        reg.put("request_send_temp_file", wilton::server::request_send_temp_file);
        reg.put("request_send_mustache", wilton::server::request_send_mustache);
        reg.put("request_send_later", wilton::server::request_send_later);
        reg.put("request_send_with_response_writer", wilton::server::request_send_with_response_writer);
        // logging
        reg.put("logging_initialize", wilton::logging::logging_initialize);
        reg.put("logging_log", wilton::logging::logging_log);
        reg.put("logging_is_level_enabled", wilton::logging::logging_is_level_enabled);
        reg.put("logging_shutdown", wilton::logging::logging_shutdown);
        // mustache
        reg.put("mustache_render", wilton::mustache::mustache_render);
        reg.put("mustache_render_file", wilton::mustache::mustache_render_file);
        //client
        reg.put("httpclient_send_request", wilton::client::httpclient_send_request);
        reg.put("httpclient_send_file", wilton::client::httpclient_send_file);
        // load
        reg.put("load_module_resource", wilton::load::load_module_resource);
        reg.put("load_module_script", wilton::load::load_module_script);
        // dyload
        reg.put("dyload_shared_library", wilton::dyload::dyload_shared_library);
        // signal
        reg.put("signal_await", wilton::signal::signal_await);
        reg.put("signal_fire", wilton::signal::signal_fire);
        // misc
        reg.put("process_spawn", wilton::misc::process_spawn);
        reg.put("get_wiltoncall_config", wilton::misc::get_wiltoncall_config);
        reg.put("stdin_readline", wilton::misc::stdin_readline);
        
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
        auto out = static_registry().invoke(call_name_str, {json_in, json_in_len_u32});
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
        static_registry().put(call_name_str, fun);
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
        static_registry().remove(call_name_str);
        return nullptr;
    } catch (const std::exception& e) {
        return wilton::support::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

#ifndef WILTON_DISABLE_DEFAULT_RUNSCRIPT

char* wiltoncall_runscript(const char* script_engine_name, int script_engine_name_len,
        const char* json_in, int json_in_len, char** json_out, int* json_out_len) {
    static std::string default_engine = wilton::internal::static_wiltoncall_config()["defaultScriptEngine"]
            .as_string("duktape");
    if (nullptr == script_engine_name) return wilton::support::alloc_copy(TRACEMSG("Null 'script_engine_name' parameter specified"));
    if (!sl::support::is_uint16(script_engine_name_len)) return wilton::support::alloc_copy(TRACEMSG(
            "Invalid 'script_engine_name_len' parameter specified: [" + sl::support::to_string(script_engine_name_len) + "]"));
    auto engine = std::ref(sl::utils::empty_string());
    uint16_t script_engine_name_len_u16 = static_cast<uint16_t> (script_engine_name_len);
    auto specified_engine = std::string(script_engine_name, script_engine_name_len_u16);
    engine = !specified_engine.empty() ? specified_engine : default_engine;
    if ("duktape" == engine.get()) {
        return wiltoncall_runscript_duktape(json_in, json_in_len, json_out, json_out_len);
    } else if ("jni" == engine.get()) {
        return wiltoncall_runscript_jni(json_in, json_in_len, json_out, json_out_len);
    }
    return wilton::support::alloc_copy(TRACEMSG("Unsupported engine: [" + engine.get() + "]"));
}

#endif // WILTON_DISABLE_DEFAULT_RUNSCRIPT

/* 
 * File:   script_engine.hpp
 * Author: alex
 *
 * Created on October 22, 2017, 7:55 PM
 */

#ifndef WILTON_SUPPORT_SCRIPT_ENGINE_HPP
#define WILTON_SUPPORT_SCRIPT_ENGINE_HPP

#include <map>
#include <mutex>
#include <string>
#include <thread>

#include "staticlib/config.hpp"
#include "staticlib/io.hpp"
#include "staticlib/json.hpp"
#include "staticlib/utils.hpp"

#include "wilton/wilton.h"
#include "wilton/wilton_loader.h"

#include "wilton/support/buffer.hpp"
#include "wilton/support/exception.hpp"
#include "wilton/support/misc.hpp"

namespace wilton {
namespace support {

namespace script_engine_map_detail {

inline sl::json::value load_wilton_config() {
    char* conf = nullptr;
    int conf_len = 0;
    auto err = wilton_config(std::addressof(conf), std::addressof(conf_len));
    if (nullptr != err) support::throw_wilton_error(err, TRACEMSG(err));
    auto deferred = sl::support::defer([conf] () STATICLIB_NOEXCEPT {
        wilton_free(conf);
    });
    const char* cconf = const_cast<const char*>(conf);
    auto json = sl::json::load({cconf, conf_len});
    return json;
}

inline sl::io::span<const char> load_init_code() {
    static const std::string code = [] {
        auto json = load_wilton_config();
        auto requirejs_dir_path = json["requireJs"]["baseUrl"].as_string_nonempty_or_throw("requireJs.baseUrl") + "/wilton-requirejs";
        auto code_path = requirejs_dir_path + "/wilton-require.js";
        char* code = nullptr;
        int code_len = 0;
        auto err_load = wilton_load_script(code_path.c_str(), static_cast<int>(code_path.length()),
                std::addressof(code), std::addressof(code_len));
        if (nullptr != err_load) {
            support::throw_wilton_error(err_load, TRACEMSG(err_load));
        }
        auto res = std::string(code, code_len);
        wilton_free(code);
        return res;
    }();
    return sl::io::make_span(code.data(), code.length());
}

inline std::string shorten_script_path(const std::string& path) {
    static sl::json::value json = load_wilton_config();
    // check stdlib path
    auto& base_url = json["requireJs"]["baseUrl"].as_string_nonempty_or_throw("requireJs.baseUrl");
    if (sl::utils::starts_with(path, base_url)) {
        auto shortened = path.substr(base_url.length());
        if (shortened.length() > 1 && '/' == shortened.at(0)) {
            return shortened.substr(1);
        }
        return shortened;
    }
    // check app paths
    auto& paths_json = json["requireJs"]["paths"];
    if (sl::json::type::object == paths_json.json_type()) {
        for (auto& fi : paths_json.as_object()) {
            const std::string& app_id = fi.name();
            const std::string& app_dir = fi.as_string_nonempty_or_throw("requireJs.paths." + app_id);
            if (sl::utils::starts_with(path, app_dir)) {
                return app_id + path.substr(app_dir.length());
            }
        }
    }
    // gave up and only strip protocol prefix
    if (sl::utils::starts_with(path, file_proto_prefix)) {
        return path.substr(file_proto_prefix.length());
    }
    return path;
}

} // namespace

template<typename Engine>
class script_engine_map {
    std::mutex mutex;
    std::map<std::string, Engine> engines;
    
public:
    support::buffer run_script(sl::io::span<const char> callback_script_json) {
        auto& en = thread_local_engine();
        return en.run_callback_script(callback_script_json);
    }

    void clean_thread_local(const char* thread_id, int thread_id_len) STATICLIB_NOEXCEPT {
        std::lock_guard<std::mutex> guard{mutex};
        if (nullptr != thread_id && sl::support::is_uint16_positive(thread_id_len)) {
            auto tid = std::string(thread_id, thread_id_len);
            engines.erase(tid);
        }
    }

private:

    // no TLS in vs2013
    Engine& thread_local_engine() {
        std::lock_guard<std::mutex> guard{mutex};
        auto tid = sl::support::to_string_any(std::this_thread::get_id());
        auto it = engines.find(tid);
        if (engines.end() == it) {
            auto code = script_engine_map_detail::load_init_code();
            auto se = Engine(code);
            auto pa = engines.insert(std::make_pair(tid, std::move(se)));
            it = pa.first;
        }
        return it->second;
    }

};

} // namespace
}


#endif /* WILTON_SUPPORT_SCRIPT_ENGINE_HPP */


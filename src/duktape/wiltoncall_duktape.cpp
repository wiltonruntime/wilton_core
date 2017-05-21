/* 
 * File:   wiltoncall_duktape.cpp
 * Author: alex
 *
 * Created on May 20, 2017, 1:17 PM
 */

#include "call/wiltoncall_internal.hpp"

#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>

#include "staticlib/config.hpp"
#include "staticlib/io.hpp"
#include "staticlib/json.hpp"
#include "staticlib/tinydir.hpp"
#include "staticlib/utils.hpp"

#include "duktape/duktape_engine.hpp"

namespace { // namespace

std::mutex& static_engines_mutex() {
    static std::mutex mutex;
    return mutex;
}

// cleaned up manually due to lack of portable TLS
std::unordered_map<std::thread::id, std::shared_ptr<wilton::duktape::duktape_engine>>& static_engines() {
    static std::unordered_map<std::thread::id, std::shared_ptr<wilton::duktape::duktape_engine>> engines;
    return engines;
}

const std::string& scripts_dir() {
    static std::string dir = [] {
        auto exepath = sl::utils::current_executable_path();
        std::replace(exepath.begin(), exepath.end(), '\\', '/');
        auto exedir = sl::utils::strip_filename(exepath);
        return exedir + "js";
    } ();
    return dir;
}

// no TLS in vs2013
std::shared_ptr<wilton::duktape::duktape_engine> thread_local_engine() {
    std::lock_guard<std::mutex> guard{static_engines_mutex()};
    auto& map = static_engines();
    auto tid = std::this_thread::get_id();
    auto it = map.find(tid);
    if (map.end() == it) {
        auto se = std::make_shared<wilton::duktape::duktape_engine>(scripts_dir());
        auto pa = map.emplace(tid, std::move(se));
        it = pa.first;
    }
    return it->second;
}

} // anonymous

char* wiltoncall_runscript_duktape(const char* json_in, int json_in_len, char** json_out,
        int* json_out_len) /* noexcept */ {
    if (nullptr == json_in) return sl::utils::alloc_copy(TRACEMSG("Null 'json_in' parameter specified"));
    if (!sl::support::is_uint32(json_in_len)) return sl::utils::alloc_copy(TRACEMSG(
            "Invalid 'json_in_len' parameter specified: [" + sl::support::to_string(json_in_len) + "]"));
    if (nullptr == json_out) return sl::utils::alloc_copy(TRACEMSG("Null 'json_out' parameter specified"));
    if (nullptr == json_out_len) return sl::utils::alloc_copy(TRACEMSG("Null 'json_out_len' parameter specified"));
    try {
        uint32_t json_in_len_u32 = static_cast<uint32_t>(json_in_len);
        auto json = std::string(json_in, json_in_len_u32);
        auto en = thread_local_engine();
        auto res = en->run_script(json);
        *json_out = sl::utils::alloc_copy(res);
        *json_out_len = res.length();
        return nullptr;
    } catch (const std::exception& e) {
        return sl::utils::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

namespace wilton {
namespace duktape {

void clean_thread_local(const std::thread::id& tid) {
    std::lock_guard<std::mutex> guard{static_engines_mutex()};
    auto& map = static_engines();
    map.erase(tid);
}

} // namespace
}

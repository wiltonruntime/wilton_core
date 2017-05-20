/* 
 * File:   wiltoncall_duktape.cpp
 * Author: alex
 *
 * Created on May 20, 2017, 1:17 PM
 */

#include "call/wiltoncall_internal.hpp"

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

// todo: think about cleanup
std::unordered_map<std::thread::id, wilton::duktape::duktape_engine>& static_engines() {
    static std::unordered_map<std::thread::id, wilton::duktape::duktape_engine> engines;
    return engines;
}

// no TLS in vs2013
wilton::duktape::duktape_engine& thread_local_engine() {
    static std::mutex mx;
    std::lock_guard<std::mutex> guard{mx};
    auto& map = static_engines();
    auto tid = std::this_thread::get_id();
    auto it = map.find(tid);
    if (map.end() == it) {
        auto pa = map.emplace(tid, wilton::duktape::duktape_engine());
        it = pa.first;
    }
    return it->second;
}

std::string read_file(const std::string& path) {
    auto src = sl::tinydir::file_source(path);
    auto sink = sl::io::string_sink();
    sl::io::copy_all(src, sink);
    return sink.get_string();
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
        auto src = sl::io::array_source(json_in, static_cast<size_t> (json_in_len));
        auto cs = sl::json::load(src);
        auto path = cs["module"].as_string_nonempty_or_throw() + ".js";
        auto code = read_file(path);
        auto& en = thread_local_engine();
        auto res = en.run_script(code, path);
        *json_out = sl::utils::alloc_copy(res);
        *json_out_len = res.length();
        return nullptr;
    } catch (const std::exception& e) {
        return sl::utils::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

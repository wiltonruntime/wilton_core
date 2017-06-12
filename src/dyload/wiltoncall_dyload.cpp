/* 
 * File:   wiltoncall_dyload.cpp
 * Author: alex
 *
 * Created on June 6, 2017, 7:26 PM
 */

#include "staticlib/config.hpp"

#ifdef STATICLIB_WINDOWS
#include "dyload/dyload_windows.hpp"
#else // !STATICLIB_WINDOWS
#include "dyload/dyload_posix.hpp"
#endif // STATICLIB_WINDOWS

#include "call/wiltoncall_internal.hpp"

#include "staticlib/utils.hpp"

namespace wilton {
namespace dyload {

sl::support::optional<sl::io::span<char>> dyload_shared_library(sl::io::span<const char> data) {
    // json parse
    auto json = sl::json::load(data);
    auto rpath = std::ref(sl::utils::empty_string());
    bool absolute = false;
    for (const sl::json::field& fi : json.as_object()) {
        auto& name = fi.name();
        if ("path" == name) {
            rpath = fi.as_string_nonempty_or_throw(name);
        } else if ("absolute" == name) {
            absolute = fi.as_bool_or_throw(name);
        } else {
            throw common::wilton_internal_exception(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (rpath.get().empty()) throw common::wilton_internal_exception(TRACEMSG(
            "Required parameter 'path' not specified"));
    const std::string& path = rpath.get();
    // call 
    std::string abspath = [&path, absolute] () -> std::string {
        if (absolute) {
            return path;
        }
        auto exepath = sl::utils::current_executable_path();
        auto exedir = sl::utils::strip_filename(exepath);
        std::replace(exedir.begin(), exedir.end(), '\\', '/');
        return std::string(exedir) + path;
    } ();
    std::function<char*()> initializer = dyload_platform(abspath);
    auto err = initializer();
    if (nullptr != err) {
        common::throw_wilton_error(err, TRACEMSG(err));
    }
    return support::empty_span();
}

} // namespace
}

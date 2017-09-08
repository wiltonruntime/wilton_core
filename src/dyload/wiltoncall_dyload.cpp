/* 
 * File:   wiltoncall_dyload.cpp
 * Author: alex
 *
 * Created on June 6, 2017, 7:26 PM
 */

#include <mutex>
#include <string>
#include <unordered_set>

#include "staticlib/config.hpp"

#ifdef STATICLIB_WINDOWS
#include "dyload/dyload_windows.hpp"
#else // !STATICLIB_WINDOWS
#include "dyload/dyload_posix.hpp"
#endif // STATICLIB_WINDOWS

#include "call/wiltoncall_internal.hpp"

#include "staticlib/utils.hpp"
#include "staticlib/tinydir/operations.hpp"

namespace wilton {
namespace dyload {

namespace { // anonymous

std::mutex& static_mutex() {
    static std::mutex mutex;
    return mutex;
}

std::unordered_set<std::string>& static_registry() {
    static std::unordered_set<std::string> set;
    return set;
}

} // namespace

support::buffer dyload_shared_library(sl::io::span<const char> data) {
    // json parse
    auto json = sl::json::load(data);
    auto rname = std::ref(sl::utils::empty_string());
    auto rdirectory = std::ref(sl::utils::empty_string());
    for (const sl::json::field& fi : json.as_object()) {
        auto& name = fi.name();
        if ("name" == name) {
            rname = fi.as_string_nonempty_or_throw(name);
        } else if ("directory" == name) {
            rdirectory = fi.as_string_nonempty_or_throw(name);
        } else {
            throw common::wilton_internal_exception(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (rname.get().empty()) throw common::wilton_internal_exception(TRACEMSG(
            "Required parameter 'path' not specified"));
    const std::string& name = rname.get();
    const std::string directory = [&rdirectory] {
        if (!rdirectory.get().empty()) {
            return rdirectory.get();
        }
        auto exepath = sl::utils::current_executable_path();
        auto exedir_raw = sl::utils::strip_filename(exepath);
        return sl::tinydir::normalize_path(exedir_raw);
    } ();
    auto key = directory + "/" + name;    
    // call
    std::lock_guard<std::mutex> guard{static_mutex()};
    if (0 == static_registry().count(key)) {
        std::function<char*()> initializer = dyload_platform(directory, name);
        auto err = initializer();
        if (nullptr != err) {
            common::throw_wilton_error(err, TRACEMSG(err));
        }
        static_registry().insert(key);
    }
    return support::make_empty_buffer();
}

} // namespace
}

/* 
 * File:   wilton_dyload.cpp
 * Author: alex
 *
 * Created on October 17, 2017, 8:13 PM
 */

#include "wilton/wilton.h"

#include <cstdint>
#include <mutex>
#include <string>

#include "staticlib/config.hpp"
#include "staticlib/json.hpp"
#include "staticlib/tinydir.hpp"
#include "staticlib/utils.hpp"

#include "wilton/support/alloc_copy.hpp"
#include "wilton/support/exception.hpp"

#ifdef STATICLIB_WINDOWS
#include "dyload/dyload_windows.hpp"
#else // !STATICLIB_WINDOWS
#include "dyload/dyload_posix.hpp"
#endif // STATICLIB_WINDOWS

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

char* wilton_dyload(const char* name, int name_len,
        const char* directory, int directory_len) /* noexcept */ {
    if (nullptr == name) return wilton::support::alloc_copy(TRACEMSG("Null 'name' parameter specified"));
    if (!sl::support::is_uint16_positive(name_len)) return wilton::support::alloc_copy(TRACEMSG(
            "Invalid 'name_len' parameter specified: [" + sl::support::to_string(name_len) + "]"));
    if (!sl::support::is_uint16(directory_len)) return wilton::support::alloc_copy(TRACEMSG(
            "Invalid 'directory_len' parameter specified: [" + sl::support::to_string(directory_len) + "]"));
    try {
        uint32_t name_len_u32 = static_cast<uint32_t> (name_len);
        auto name_str = std::string(name, name_len_u32);

        // find out directory
        const std::string directory_str = [directory, directory_len] () -> std::string {
            if (nullptr != directory && directory_len > 0) {
                return std::string(directory, static_cast<uint16_t>(directory_len));
            }
            auto exepath = sl::utils::current_executable_path();
            auto exedir_raw = sl::utils::strip_filename(exepath);
            return sl::tinydir::normalize_path(exedir_raw);
        } ();

        // call
        std::lock_guard<std::mutex> guard{static_mutex()};
        if (0 == static_registry().count(name)) {
            std::function<char*()> initializer = wilton::dyload::dyload_platform(directory_str, name_str);
            auto err = initializer();
            if (nullptr != err) {
                wilton::support::throw_wilton_error(err, TRACEMSG(err));
            }
            static_registry().insert(name);
        }
        
        return nullptr;
    } catch (const std::exception& e) {
        return wilton::support::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

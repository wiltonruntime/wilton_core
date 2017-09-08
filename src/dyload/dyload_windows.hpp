/* 
 * File:   dyload_windows.hpp
 * Author: alex
 *
 * Created on June 6, 2017, 7:32 PM
 */

#ifndef WILTON_DYLOAD_WINDOWS_HPP
#define	WILTON_DYLOAD_WINDOWS_HPP

#include <functional>
#include <string>

#ifndef UNICODE
#define UNICODE
#endif // UNICODE
#ifndef _UNICODE
#define _UNICODE
#endif // _UNICODE
#ifndef NOMINMAX
#define NOMINMAX
#endif NOMINMAX
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "staticlib/config.hpp"
#include "staticlib/utils.hpp"

#include "call/wiltoncall_internal.hpp"

namespace wilton {
namespace dyload {

std::function<char*()> dyload_platform(const std::string& directory, const std::string& name) {
    auto absolute_path = directory + "/" + name + ".dll";
    auto wpath = sl::utils::widen(absolute_path);
    auto handle = ::LoadLibraryW(wpath.c_str());
    if (nullptr == handle) {
        throw common::wilton_internal_exception(TRACEMSG(
            "Error loading shared library on path: [" + absolute_path + "],"
            " error: [" + sl::utils::errcode_to_string(::GetLastError()) + "]"));
    }
    auto initter = ::GetProcAddress(handle, "wilton_module_init");
    if (nullptr == initter) {
        throw common::wilton_internal_exception(TRACEMSG(
            "Error loading 'wilton_module_init' from shared library on path: [" + absolute_path + "],"
            " error: [" + sl::utils::errcode_to_string(::GetLastError()) + "]"));
    }
    return [initter]() {
        auto fun = reinterpret_cast<char*(*)()>(initter);
        return fun();
    };
}

} // namespace
}

#endif	/* WILTON_DYLOAD_WINDOWS_HPP */


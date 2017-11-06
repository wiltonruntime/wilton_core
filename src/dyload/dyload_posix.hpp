/* 
 * File:   dyload_posix.hpp
 * Author: alex
 *
 * Created on June 6, 2017, 7:33 PM
 */

#ifndef WILTON_DYLOAD_POSIX_HPP
#define WILTON_DYLOAD_POSIX_HPP

#include <functional>

#include <dlfcn.h>

#include "staticlib/config.hpp"

#include "call/wiltoncall_internal.hpp"

namespace wilton {
namespace dyload {

namespace { // anonymous

std::string dlerr_str() {
    auto res = ::dlerror();
    return nullptr != res ? std::string(res) : "";
}

} // namespace

std::function<char*()> dyload_platform(const std::string& directory, const std::string& name) {
#ifdef STATICLIB_MAC
    auto absolute_path = directory + "/lib" + name + ".dylib";
#else // !STATICLIB_MAC
    auto absolute_path = directory + "/lib" + name + ".so";
#endif
    auto handle = ::dlopen(absolute_path.c_str(), RTLD_LAZY);
    if (nullptr == handle) {
        throw support::exception(TRACEMSG(
                "Error loading shared library on path: [" + absolute_path + "],"
                " error: [" + dlerr_str() + "]"));
    }
    auto initter = ::dlsym(handle, "wilton_module_init");
    if (nullptr == initter) {
        throw support::exception(TRACEMSG(
                "Error loading 'wilton_module_init' from shared library on path: [" + absolute_path + "],"
                " error: [" + dlerr_str() + "]"));
    }
    return [initter]() {
        auto fun = reinterpret_cast<char*(*)()>(initter);
        return fun();
    };
}

} // namespace
}

#endif /* WILTON_DYLOAD_POSIX_HPP */


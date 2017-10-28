/* 
 * File:   wiltoncall_registry.hpp
 * Author: alex
 *
 * Created on January 8, 2017, 4:48 PM
 */

#ifndef WILTON_CALL_WILTONCALL_REGISTRY_HPP
#define WILTON_CALL_WILTONCALL_REGISTRY_HPP

#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>

#include "staticlib/json.hpp"
#include "staticlib/io.hpp"
#include "staticlib/support.hpp"

#include "wilton/support/buffer.hpp"
#include "wilton/support/exception.hpp"

namespace wilton {
namespace call {

namespace { // anonymous

using fun_type = std::function<support::buffer(sl::io::span<const char> data)>;
using map_type = std::unordered_map<std::string, fun_type>;

} // namespace

class wiltoncall_registry {
    std::mutex mutex;
    map_type registry;

public:
    void put(const std::string& name, fun_type callback) {
        std::lock_guard<std::mutex> guard{mutex};
        if (name.empty()) {
            throw support::exception(TRACEMSG("Invalid empty wilton_function name specified"));
        }
        // todo: check override
        auto pa = registry.emplace(name, callback);
        if (!pa.second) {
            throw support::exception(TRACEMSG(
                    "Invalid duplicate wilton_function name specified: [" + name + "]"));
        }
    }

    void remove(const std::string& name) {
        std::lock_guard<std::mutex> guard{mutex};
        if (name.empty()) {
            throw support::exception(TRACEMSG("Invalid empty wilton_function name specified"));
        }
        auto res = registry.erase(name);
        if (0 == res) {
            throw support::exception(TRACEMSG(
                    "Invalid unknown wilton_function name specified: [" + name + "]"));
        }
    }

    support::buffer invoke(const std::string& name, sl::io::span<const char> data) {
        if (name.empty()) {
            throw support::exception(TRACEMSG("Invalid empty wilton_function name specified"));
        }
        try {
            // get function
            fun_type fun = [](sl::io::span<const char>) {
                return support::make_empty_buffer();
            };
            {
                std::lock_guard<std::mutex> guard{mutex};
                auto it = registry.find(name);
                if (registry.end() == it) {
                    throw support::exception(TRACEMSG(
                            "Invalid unknown wilton_function name specified: [" + name + "]"));
                }
                fun = it->second;
            }
            // invoke
            return fun(data);
        } catch (const std::exception& e) {
            throw support::exception(TRACEMSG(e.what() + 
                    "\n'wiltoncall' error for function: [" + name + "]"));
        }
    }

};

} // namespace
}

#endif /* WILTON_CALL_WILTONCALL_REGISTRY_HPP */


/* 
 * File:   wiltoncall_registry.hpp
 * Author: alex
 *
 * Created on January 8, 2017, 4:48 PM
 */

#ifndef WILTON_CALL_WILTONCALL_REGISTRY_HPP
#define	WILTON_CALL_WILTONCALL_REGISTRY_HPP

#include <mutex>
#include <string>
#include <unordered_map>

#include "staticlib/json.hpp"

#include "common/wilton_internal_exception.hpp"

namespace wilton {
namespace call {

namespace { // anonymous

using fun_type = std::function<std::string(const std::string&)>;
using map_type = std::unordered_map<std::string, fun_type>;

} // namespace

class wiltoncall_registry {
    std::mutex mutex;
    map_type registry;
    
public:
    void put(const std::string& name, fun_type callback) {
        std::lock_guard<std::mutex> guard{mutex};
        if (name.empty()) {
            throw common::wilton_internal_exception(TRACEMSG("Invalid empty wilton_function name specified"));
        }
        auto pa = registry.emplace(name, callback);
        if (!pa.second) {
            throw common::wilton_internal_exception(TRACEMSG(
                    "Invalid duplicate wilton_function name specified: [" + name + "]"));
        }
    }
    
    void remove(const std::string& name) {
        std::lock_guard<std::mutex> guard{mutex};
        if (name.empty()) {
            throw common::wilton_internal_exception(TRACEMSG("Invalid empty wilton_function name specified"));
        }
        auto res = registry.erase(name);
        if (0 == res) {
            throw common::wilton_internal_exception(TRACEMSG(
                    "Invalid unknown wilton_function name specified: [" + name + "]"));
        }
    }
    
    std::string invoke(const std::string& name, const std::string& data) {
        if (name.empty()) {
            throw common::wilton_internal_exception(TRACEMSG("Invalid empty wilton_function name specified"));
        }
        try {
            // get function
            fun_type fun = [](const std::string&) {
                return std::string();
            };
            {
                std::lock_guard<std::mutex> guard{mutex};
                auto it = registry.find(name);
                if (registry.end() == it) {
                    throw common::wilton_internal_exception(TRACEMSG(
                            "Invalid unknown wilton_function name specified: [" + name + "]"));
                }
                fun = it->second;
            }
            // invoke
            return fun(data);
        } catch (const std::exception& e) {
            throw common::wilton_internal_exception(TRACEMSG(e.what() + 
                    "\nwiltoncall error for function: [" + name + "]"));
        }
    }

};

} // namespace
}

#endif	/* WILTON_CALL_WILTONCALL_REGISTRY_HPP */


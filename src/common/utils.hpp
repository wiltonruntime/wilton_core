/* 
 * File:   utils.hpp
 * Author: alex
 *
 * Created on June 14, 2016, 11:45 AM
 */

#ifndef WILTON_COMMON_UTILS_HPP
#define	WILTON_COMMON_UTILS_HPP

#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "staticlib/json.hpp"

#include "common/wilton_internal_exception.hpp"

namespace wilton {
namespace common {

void throw_wilton_error(char* err, const std::string& msg);

std::string wrap_wilton_output(char* out, int out_len);

void check_json_callback_script(const sl::json::field& field);

void dump_error(const std::string& directory, const std::string& msg);


template<typename T>
class handle_registry {
    std::unordered_set<T*> registry;
    std::mutex mutex;

public:
    int64_t put(T* ptr) {
        std::lock_guard<std::mutex> lock{mutex};
        auto pair = registry.insert(ptr);
        return pair.second ? reinterpret_cast<int64_t> (ptr) : 0;
    }

    T* remove(int64_t handle) {
        std::lock_guard<std::mutex> lock{mutex};
        T* ptr = reinterpret_cast<T*> (handle);
        auto erased = registry.erase(ptr);
        return 1 == erased ? ptr : nullptr;
    }

    T* peek(int64_t handle) {
        std::lock_guard<std::mutex> lock{mutex};
        T* ptr = reinterpret_cast<T*> (handle);
        auto exists = registry.count(ptr);
        return 1 == exists ? ptr : nullptr;
    }
};

template<typename T, typename P>
class payload_handle_registry {
    std::unordered_map<T*, P> registry;
    std::mutex mutex;

public:
    int64_t put(T* ptr, P&& ctx) {
        std::lock_guard<std::mutex> lock(mutex);
        auto pair = registry.emplace(ptr, std::move(ctx));
        return pair.second ? reinterpret_cast<int64_t> (ptr) : 0;
    }

    std::pair<T*, P> remove(int64_t handle) {
        std::lock_guard<std::mutex> lock(mutex);
        T* ptr = reinterpret_cast<T*> (handle);
        auto it = registry.find(ptr);
        if (registry.end() != it) {
            auto ctx = std::move(it->second);
            registry.erase(ptr);
            return std::make_pair(ptr, std::move(ctx));
        } else {
            return std::make_pair(nullptr, P());
        }
    }
};

} //namespace
}        
        
#endif	/* WILTON_COMMON_UTILS_HPP */


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
#include <unordered_set>
#include <vector>

#include "staticlib/serialization.hpp"

#include "common/WiltonInternalException.hpp"

namespace wilton {
namespace common {

const std::string& empty_string();

void throw_wilton_error(char* err, const std::string& msg);

const std::string& get_json_string(const staticlib::serialization::JsonField& field);

int64_t get_json_int64(const staticlib::serialization::JsonField& field);

uint32_t get_json_uint32(const staticlib::serialization::JsonField& field);

uint16_t get_json_uint16(const staticlib::serialization::JsonField& field);

bool get_json_bool(const staticlib::serialization::JsonField& field);

const std::vector<staticlib::serialization::JsonValue>& get_json_array(
        const staticlib::serialization::JsonField& field);

const std::vector<staticlib::serialization::JsonField>& get_json_object(
        const staticlib::serialization::JsonField& field);

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

} //namespace
}        
        
#endif	/* WILTON_COMMON_UTILS_HPP */


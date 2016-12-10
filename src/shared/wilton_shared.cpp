/* 
 * File:   wilton_shared.cpp
 * Author: alex
 *
 * Created on December 9, 2016, 6:50 PM
 */

#include "wilton/wilton.h"

#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <ios>

#include "staticlib/config.hpp"
#include "staticlib/utils.hpp"

namespace { // anonymous

namespace sc = staticlib::config;
namespace su = staticlib::utils;

class Value {
    std::string value;
    std::shared_ptr<std::condition_variable> cv;

public:
    Value(std::string&& value) :
    value(std::move(value)),
    cv(std::shared_ptr<std::condition_variable>(new std::condition_variable())) { }
    
    Value(const Value&) = delete;
    
    Value& operator=(const Value&) = delete;
    
    Value(Value&& other) :
    value(std::move(other.value)),
    cv(std::move(other.cv)) { }
    
    Value& operator=(Value&&) = delete;
    
    const std::string& get_value() const {
        return value;
    }
    
    std::shared_ptr<std::condition_variable> get_cv() const {
        return cv;
    }
    
    std::condition_variable& get_cv_ref() {
        return *cv;
    }
};

std::mutex& static_mutex() {
    static std::mutex mutex;
    return mutex;
}

std::unordered_map<std::string, Value>& static_map() {
    static std::unordered_map<std::string, Value> map;
    return map;
}

} // namespace

char* wilton_shared_put(const char* key, int key_len, const char* value, int value_len) /* noexcept */ {
    if (nullptr == key) return su::alloc_copy(TRACEMSG("Null 'key' parameter specified"));
    if (!su::is_positive_uint16(key_len)) return su::alloc_copy(TRACEMSG(
            "Invalid 'key_len' parameter specified: [" + sc::to_string(key_len) + "]"));
    if (nullptr == value) return su::alloc_copy(TRACEMSG("Null 'value' parameter specified"));
    if (!su::is_uint32(value_len)) return su::alloc_copy(TRACEMSG(
            "Invalid 'value_len' parameter specified: [" + sc::to_string(value_len) + "]"));
    try {
        // prepare tuple
        uint16_t key_len_u16 = static_cast<uint16_t>(key_len);
        std::string key_str{key, key_len_u16};
        uint32_t value_len_u32 = static_cast<uint32_t>(value_len);
        std::string value_str{value, value_len_u32};
        // notify waiters, remove existing, put new
        std::lock_guard<std::mutex> guard{static_mutex()};
        auto it = static_map().find(key_str);
        if (static_map().end() != it) {
            it->second.get_cv_ref().notify_all();
            static_map().erase(key_str);
        }
        static_map().emplace(std::move(key_str), Value(std::move(value_str)));
        return nullptr;
    } catch (const std::exception& e) {
        return su::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

char* wilton_shared_get(const char* key, int key_len, char** value_out, int* value_out_len) /* noexcept */ {
    if (nullptr == key) return su::alloc_copy(TRACEMSG("Null 'key' parameter specified"));
    if (!su::is_positive_uint16(key_len)) return su::alloc_copy(TRACEMSG(
            "Invalid 'key_len' parameter specified: [" + sc::to_string(key_len) + "]"));
    if (nullptr == value_out) return su::alloc_copy(TRACEMSG("Null 'value_out' parameter specified"));
    if (nullptr == value_out_len) return su::alloc_copy(TRACEMSG("Null 'value_out_len' parameter specified"));
    try {
        // prepare key
        uint16_t key_len_u16 = static_cast<uint16_t> (key_len);
        std::string key_str{key, key_len_u16};
        // lock and search
        std::lock_guard<std::mutex> guard{static_mutex()};
        auto it = static_map().find(key_str);
        if (static_map().end() != it) {
            *value_out = su::alloc_copy(it->second.get_value());
            *value_out_len = static_cast<int>(it->second.get_value().length());
        } else {
            *value_out = nullptr;
            *value_out_len = -1;
        }
        return nullptr;
    } catch (const std::exception& e) {
        return su::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

// multiple searches over map are okay - map not going to be large
char* wilton_shared_wait_change(int timeout_millis, const char* key, int key_len,
        const char* current_value, int current_value_len, char** changed_value_out,
        int* changed_value_out_len) /* noexcept */ {
    if (!su::is_positive_uint32(timeout_millis)) return su::alloc_copy(TRACEMSG(
            "Invalid 'timeout_millis' parameter specified: [" + sc::to_string(timeout_millis) + "]"));
    if (nullptr == key) return su::alloc_copy(TRACEMSG("Null 'key' parameter specified"));
    if (!su::is_positive_uint16(key_len)) return su::alloc_copy(TRACEMSG(
            "Invalid 'key_len' parameter specified: [" + sc::to_string(key_len) + "]"));
    if (nullptr == current_value) return su::alloc_copy(TRACEMSG("Null 'current_value' parameter specified"));
    if (!su::is_uint32(current_value_len)) return su::alloc_copy(TRACEMSG(
            "Invalid 'current_value_len' parameter specified: [" + sc::to_string(current_value_len) + "]"));
    if (nullptr == changed_value_out) return su::alloc_copy(TRACEMSG("Null 'changed_value_out' parameter specified"));
    if (nullptr == changed_value_out_len) return su::alloc_copy(TRACEMSG("Null 'changed_value_out_len' parameter specified"));
    try {
        // prepare tuple
        uint32_t timeout_millis_u32 = static_cast<uint32_t> (timeout_millis);
        uint16_t key_len_u16 = static_cast<uint16_t> (key_len);
        std::string key_str{key, key_len_u16};
        uint32_t current_value_len_u32 = static_cast<uint32_t> (current_value_len);
        std::string current_value_str{current_value, current_value_len_u32};
        // lock find and wait
        std::unique_lock<std::mutex> lock{static_mutex()};
        auto it = static_map().find(key_str);
        if (static_map().end() == it) {
            return su::alloc_copy(TRACEMSG("Shared record not found, key: [" + key_str + "]"));
        }
        const std::string& val = it->second.get_value();
        if (val != current_value_str) {
            *changed_value_out = su::alloc_copy(val);
            *changed_value_out_len = val.length();
            return nullptr;
        }
        *changed_value_out = nullptr;
        *changed_value_out_len = -1;
        auto cv = it->second.get_cv();
        cv->wait_for(lock, std::chrono::milliseconds(timeout_millis_u32),
                [&key_str, &current_value_str, changed_value_out, changed_value_out_len](){
            auto it = static_map().find(key_str);
            if (static_map().end() == it) {
                return true;
            }
            if (it->second.get_value() != current_value_str) {
                *changed_value_out = su::alloc_copy(it->second.get_value());
                *changed_value_out_len = static_cast<int> (it->second.get_value().length());
                return true;
            }
            return false;
        });
        if (nullptr != *changed_value_out) {
            return nullptr;
        } else {
            return su::alloc_copy(TRACEMSG("Shared record wait timeout exceeded," +
                    "key: [" + key_str + "], timeout_millis: [" + sc::to_string(timeout_millis_u32) + "]"));
        }
    } catch (const std::exception& e) {
        return su::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

char* wilton_shared_remove(const char* key, int key_len) /* noexcept */ {
    if (nullptr == key) return su::alloc_copy(TRACEMSG("Null 'key' parameter specified"));
    if (!su::is_positive_uint16(key_len)) return su::alloc_copy(TRACEMSG(
            "Invalid 'key_len' parameter specified: [" + sc::to_string(key_len) + "]"));
    try {
        // prepare key
        uint16_t key_len_u16 = static_cast<uint16_t> (key_len);
        std::string key_str{key, key_len_u16};
        // lock, notify and remove
        std::lock_guard<std::mutex> guard{static_mutex()};
        auto it = static_map().find(key_str);
        if (static_map().end() != it) {
            it->second.get_cv_ref().notify_all();
            static_map().erase(key_str);
        }
        return nullptr;
    } catch (const std::exception& e) {
        return su::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

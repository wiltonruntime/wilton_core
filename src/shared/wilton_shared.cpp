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
#include "staticlib/json.hpp"
#include "staticlib/utils.hpp"

#include "wilton/support/alloc_copy.hpp"
#include "wilton/support/buffer.hpp"

namespace { // anonymous

class shared_entry {
    std::shared_ptr<std::condition_variable> cv;

public:
    std::string value;
    
    shared_entry(std::string&& value) :
    cv(std::shared_ptr<std::condition_variable>(new std::condition_variable())),
    value(std::move(value)) { }
    
    shared_entry(const shared_entry&) = delete;
    
    shared_entry& operator=(const shared_entry&) = delete;
    
    shared_entry(shared_entry&& other) :
    cv(std::move(other.cv)),
    value(std::move(other.value)) { }
    
    shared_entry& operator=(shared_entry&&) = delete;
    
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

std::unordered_map<std::string, shared_entry>& static_map() {
    static std::unordered_map<std::string, shared_entry> map;
    return map;
}

} // namespace

char* wilton_shared_put(const char* key, int key_len, const char* value, int value_len,
        char** prev_value_out, int* prev_value_out_len) /* noexcept */ {
    if (nullptr == key) return wilton::support::alloc_copy(TRACEMSG("Null 'key' parameter specified"));
    if (!sl::support::is_uint16_positive(key_len)) return wilton::support::alloc_copy(TRACEMSG(
            "Invalid 'key_len' parameter specified: [" + sl::support::to_string(key_len) + "]"));
    if (nullptr == value) return wilton::support::alloc_copy(TRACEMSG("Null 'value' parameter specified"));
    if (!sl::support::is_uint32(value_len)) return wilton::support::alloc_copy(TRACEMSG(
            "Invalid 'value_len' parameter specified: [" + sl::support::to_string(value_len) + "]"));
    if (nullptr == prev_value_out) return wilton::support::alloc_copy(TRACEMSG("Null 'prev_value_out' parameter specified"));
    if (nullptr == prev_value_out_len) return wilton::support::alloc_copy(TRACEMSG("Null 'prev_value_out_len' parameter specified"));
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
            *prev_value_out = wilton::support::alloc_copy(it->second.value);
            *prev_value_out_len = static_cast<int> (it->second.value.length());
            it->second.get_cv_ref().notify_all();
            static_map().erase(key_str);
        } else {
            *prev_value_out = nullptr;
            *prev_value_out_len = -1;
        }
        static_map().emplace(std::move(key_str), shared_entry(std::move(value_str)));
        return nullptr;
    } catch (const std::exception& e) {
        return wilton::support::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

char* wilton_shared_get(const char* key, int key_len, char** value_out, int* value_out_len) /* noexcept */ {
    if (nullptr == key) return wilton::support::alloc_copy(TRACEMSG("Null 'key' parameter specified"));
    if (!sl::support::is_uint16_positive(key_len)) return wilton::support::alloc_copy(TRACEMSG(
            "Invalid 'key_len' parameter specified: [" + sl::support::to_string(key_len) + "]"));
    if (nullptr == value_out) return wilton::support::alloc_copy(TRACEMSG("Null 'value_out' parameter specified"));
    if (nullptr == value_out_len) return wilton::support::alloc_copy(TRACEMSG("Null 'value_out_len' parameter specified"));
    try {
        // prepare key
        uint16_t key_len_u16 = static_cast<uint16_t> (key_len);
        std::string key_str{key, key_len_u16};
        // lock and search
        std::lock_guard<std::mutex> guard{static_mutex()};
        auto it = static_map().find(key_str);
        if (static_map().end() != it) {
            *value_out = wilton::support::alloc_copy(it->second.value);
            *value_out_len = static_cast<int>(it->second.value.length());
        } else {
            *value_out = nullptr;
            *value_out_len = -1;
        }
        return nullptr;
    } catch (const std::exception& e) {
        return wilton::support::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

// multiple searches over map are okay - map not going to be large
char* wilton_shared_wait_change(int timeout_millis, const char* key, int key_len,
        const char* current_value, int current_value_len, char** changed_value_out,
        int* changed_value_out_len) /* noexcept */ {
    if (!sl::support::is_uint32_positive(timeout_millis)) return wilton::support::alloc_copy(TRACEMSG(
            "Invalid 'timeout_millis' parameter specified: [" + sl::support::to_string(timeout_millis) + "]"));
    if (nullptr == key) return wilton::support::alloc_copy(TRACEMSG("Null 'key' parameter specified"));
    if (!sl::support::is_uint16_positive(key_len)) return wilton::support::alloc_copy(TRACEMSG(
            "Invalid 'key_len' parameter specified: [" + sl::support::to_string(key_len) + "]"));
    if (nullptr == current_value) return wilton::support::alloc_copy(TRACEMSG("Null 'current_value' parameter specified"));
    if (!sl::support::is_uint32(current_value_len)) return wilton::support::alloc_copy(TRACEMSG(
            "Invalid 'current_value_len' parameter specified: [" + sl::support::to_string(current_value_len) + "]"));
    if (nullptr == changed_value_out) return wilton::support::alloc_copy(TRACEMSG("Null 'changed_value_out' parameter specified"));
    if (nullptr == changed_value_out_len) return wilton::support::alloc_copy(TRACEMSG("Null 'changed_value_out_len' parameter specified"));
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
            return wilton::support::alloc_copy(TRACEMSG("Shared record not found, key: [" + key_str + "]"));
        }
        const std::string& val = it->second.value;
        if (val != current_value_str) {
            *changed_value_out = wilton::support::alloc_copy(val);
            *changed_value_out_len = static_cast<int>(val.length());
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
            if (it->second.value != current_value_str) {
                *changed_value_out = wilton::support::alloc_copy(it->second.value);
                *changed_value_out_len = static_cast<int> (it->second.value.length());
                return true;
            }
            return false;
        });
        if (nullptr != *changed_value_out) {
            return nullptr;
        } else {
            return wilton::support::alloc_copy(TRACEMSG("Shared record wait timeout exceeded," +
                    "key: [" + key_str + "], timeout_millis: [" + sl::support::to_string(timeout_millis_u32) + "]"));
        }
    } catch (const std::exception& e) {
        return wilton::support::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

char* wilton_shared_remove(const char* key, int key_len, char** value_out, int* value_out_len) /* noexcept */ {
    if (nullptr == key) return wilton::support::alloc_copy(TRACEMSG("Null 'key' parameter specified"));
    if (!sl::support::is_uint16_positive(key_len)) return wilton::support::alloc_copy(TRACEMSG(
            "Invalid 'key_len' parameter specified: [" + sl::support::to_string(key_len) + "]"));
    if (nullptr == value_out) return wilton::support::alloc_copy(TRACEMSG("Null 'value_out' parameter specified"));
    if (nullptr == value_out_len) return wilton::support::alloc_copy(TRACEMSG("Null 'value_out_len' parameter specified"));    
    try {
        // prepare key
        uint16_t key_len_u16 = static_cast<uint16_t> (key_len);
        std::string key_str{key, key_len_u16};
        // lock, notify and remove
        std::lock_guard<std::mutex> guard{static_mutex()};
        auto it = static_map().find(key_str);
        if (static_map().end() != it) {
            *value_out = wilton::support::alloc_copy(it->second.value);
            *value_out_len = static_cast<int> (it->second.value.length());
            it->second.get_cv_ref().notify_all();
            static_map().erase(key_str);
        } else {
            *value_out = nullptr;
            *value_out_len = -1;
        }
        return nullptr;
    } catch (const std::exception& e) {
        return wilton::support::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

char* wilton_shared_list_append(const char* key, int key_len, const char* value, int value_len,
        char** updated_value_out, int* updated_value_out_len) /* noexcept */ {
    if (nullptr == key) return wilton::support::alloc_copy(TRACEMSG("Null 'key' parameter specified"));
    if (!sl::support::is_uint16_positive(key_len)) return wilton::support::alloc_copy(TRACEMSG(
            "Invalid 'key_len' parameter specified: [" + sl::support::to_string(key_len) + "]"));
    if (nullptr == value) return wilton::support::alloc_copy(TRACEMSG("Null 'value' parameter specified"));
    if (!sl::support::is_uint32(value_len)) return wilton::support::alloc_copy(TRACEMSG(
            "Invalid 'value_len' parameter specified: [" + sl::support::to_string(value_len) + "]"));
    if (nullptr == updated_value_out) return wilton::support::alloc_copy(TRACEMSG("Null 'updated_value_out' parameter specified"));
    if (nullptr == updated_value_out_len) return wilton::support::alloc_copy(TRACEMSG("Null 'updated_value_out_len' parameter specified"));
    try {
        // prepare tuple
        uint16_t key_len_u16 = static_cast<uint16_t> (key_len);
        auto key_str = std::string(key, key_len_u16);
        auto value_json = sl::json::load({value, value_len});
        // append and notify
        std::lock_guard<std::mutex> guard{static_mutex()};
        auto it = static_map().find(key_str);
        if (static_map().end() != it) {
            auto& entry = it->second;
            auto list_json = sl::json::loads(entry.value);
            auto& list = list_json.as_array_or_throw(TRACEMSG(
                    "Invalid non-array existing value found, key: [" + key + "]," +
                    " value: [" + it->second.value + "]"));
            list.emplace_back(std::move(value_json));
            entry.value = list_json.dumps();
            *updated_value_out = wilton::support::alloc_copy(entry.value);
            *updated_value_out_len = static_cast<int> (entry.value.length());
            entry.get_cv_ref().notify_all();
        } else {
            auto vec = std::vector<sl::json::value>();
            vec.emplace_back(std::move(value_json));
            auto value_str = sl::json::dumps(std::move(vec));
            *updated_value_out = wilton::support::alloc_copy(value_str);
            *updated_value_out_len = static_cast<int> (value_str.length());
            static_map().emplace(std::move(key_str), shared_entry(std::move(value_str)));
        }
        return nullptr;
    } catch (const std::exception& e) {
        return wilton::support::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

char* wilton_shared_dump(char** dump_out, int* dump_out_len) {
    if (nullptr == dump_out) return wilton::support::alloc_copy(TRACEMSG("Null 'dump_out' parameter specified"));
    if (nullptr == dump_out_len) return wilton::support::alloc_copy(TRACEMSG("Null 'dump_out_len' parameter specified"));
    try {
        std::lock_guard<std::mutex> guard{static_mutex()};
        auto res = std::vector<sl::json::field> ();
        for (auto& pa : static_map()) {
            res.emplace_back(pa.first, pa.second.value);
        }
        auto span = wilton::support::make_json_buffer(sl::json::value(std::move(res)));
        *dump_out = span.value().data();
        *dump_out_len = static_cast<int> (span.value().size());
        return nullptr;
    } catch (const std::exception& e) {
        return wilton::support::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

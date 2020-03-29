/*
 * Copyright 2019, alex at staticlibs.net
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* 
 * File:   wiltoncall_registry.hpp
 * Author: alex
 *
 * Created on March 18, 2019, 11:17 AM
 */

#ifndef WILTON_CALL_WILTONCALL_REGISTRY_HPP
#define WILTON_CALL_WILTONCALL_REGISTRY_HPP

#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <string>
#include <utility>
#include <unordered_map>
#include <unordered_set>

#include "staticlib/config.hpp"
#include "staticlib/json.hpp"
#include "staticlib/io.hpp"
#include "staticlib/support.hpp"

#include "wilton/support/exception.hpp"

namespace wilton {
namespace internal {

using cb_ctx_type = void*;
using cb_fun_type = char* (*)(void* call_ctx, const char* json_in, int json_in_len, char** json_out, int* json_out_len);

class wiltoncall_registry {
    size_t max_reg_entries;
    std::mutex mutex;
    std::map<std::string, std::pair<cb_ctx_type, cb_fun_type>> map;
    // no thread-locals in VS2013
    // todo: consider using thread::id instead of a string
    std::unordered_map<std::string, std::unordered_set<std::string>> caps_tlmap;

public:
    wiltoncall_registry(size_t max_registry_entries_count):
    max_reg_entries(max_registry_entries_count) { }

    wiltoncall_registry(const wiltoncall_registry& other) = delete;

    wiltoncall_registry& operator=(const wiltoncall_registry& other) = delete;

    void put(const std::string& name, cb_ctx_type cb_ctx, cb_fun_type cb_fun) {
        if (name.empty()) throw wilton::support::exception(TRACEMSG(
                "Invalid empty 'wiltoncall' name specified"));
        if (nullptr == cb_fun) throw wilton::support::exception(TRACEMSG(
                "Invalid null 'wiltoncall' function specified for name: [" + name + "]"));
        std::lock_guard<std::mutex> guard{mutex};
        if (map.size() >= max_reg_entries) throw wilton::support::exception(TRACEMSG(
                "'wiltoncall' wiltoncall_registry size exceeded, max size: [" + sl::support::to_string(max_reg_entries) + "]"));
        if (0 == map.count(name)) {
            map.insert(std::make_pair(name, std::make_pair(cb_ctx, cb_fun)));
        } else {
            throw wilton::support::exception(TRACEMSG(
                    "Invalid duplicate 'wiltoncall' name specified: [" + name + "]"));
        }
    }

    std::pair<cb_ctx_type, cb_fun_type> get(const std::string& name) {
        if (name.empty()) throw wilton::support::exception(TRACEMSG(
                "Invalid empty 'wiltoncall' name specified"));
        std::lock_guard<std::mutex> guard{mutex};
        auto it = map.find(name);
        if (map.end() == it) {
            throw wilton::support::exception(TRACEMSG(
                    "Invalid unknown 'wiltoncall' name specified: [" + name + "]"));
        }
        // capabilities check
        auto tid = sl::support::to_string_any(std::this_thread::get_id());
        auto cit = caps_tlmap.find(tid);
        if (caps_tlmap.end() != cit) {
            if (0 == cit->second.count(name)) {
                auto lcaps = std::vector<sl::json::value>();
                for(const std::string& cp : cit->second) {
                    lcaps.push_back(cp);
                }
                auto lval = sl::json::value(std::move(lcaps));
                auto lst = sl::json::dumps(lval);
                throw wilton::support::exception(TRACEMSG(
                        "Wilton capabilities error," + 
                        " thread id: [" + tid + "]," +
                        " call name: [" + name +"]," +
                        " capabilities: [" + lst + "]"));
            }
        }
        return it->second;
    }

    std::vector<std::string> list() {
        std::lock_guard<std::mutex> guard{mutex};
        auto vec = std::vector<std::string>();
        for (auto& en : map) {
            vec.push_back(en.first);
        }
        return vec;
    }

    void remove(const std::string& name) {
        if (name.empty()) throw wilton::support::exception(TRACEMSG(
                "Invalid empty 'wiltoncall' name specified"));
        std::lock_guard<std::mutex> guard{mutex};
        auto res = map.erase(name);
        if (0 == res) {
            throw wilton::support::exception(TRACEMSG(
                    "Invalid unknown 'wiltoncall' name specified: [" + name + "]"));
        }
    }

    void set_thread_caps(std::unordered_set<std::string> caps) {
        std::lock_guard<std::mutex> guard{mutex};
        auto tid = sl::support::to_string_any(std::this_thread::get_id());
        if (0 != caps_tlmap.count(tid)) {
            throw wilton::support::exception(TRACEMSG(
                    "Thread capabilities already set, thread id: [" + tid + "]"));
        }
        for (auto& cp : caps) {
            if (0 == map.count(cp)) {
                throw wilton::support::exception(TRACEMSG(
                        "Thread capabilities already set, thread id: [" + tid + "]"));
            }
        }
        caps_tlmap.insert(std::make_pair(tid, std::move(caps)));
    }
};

} // namespace
} // namespace

#endif /* WILTON_CALL_WILTONCALL_REGISTRY_HPP */


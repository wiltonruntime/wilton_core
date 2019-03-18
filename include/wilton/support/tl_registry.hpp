/*
 * Copyright 2018, alex at staticlibs.net
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
 * File:   tl_registry.hpp
 * Author: alex
 *
 * Created on May 16, 2018, 12:19 PM
 */

#ifndef WILTON_SUPPORT_TL_REGISTRY_HPP
#define WILTON_SUPPORT_TL_REGISTRY_HPP

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "staticlib/json.hpp"
#include "staticlib/json.hpp"

#include "wilton/support/exception.hpp"

namespace wilton {
namespace support {

template<typename T>
class tl_registry {
    std::mutex mtx;
    std::map<std::string, T> map;

public:
    void put(T&& value) {
        auto tid = sl::support::to_string_any(std::this_thread::get_id());
        std::lock_guard<std::mutex> guard{mtx};
        auto res = map.insert(std::make_pair(tid, std::move(value)));
        if (false == res.second) {
            throw wilton::support::exception(TRACEMSG("Invalid duplicate TL entry to put," + 
                    " tid: [" + tid + "], keys: [" + dump() + "]"));
        }
    }

    T& peek() {
        auto tid = sl::support::to_string_any(std::this_thread::get_id());
        std::lock_guard<std::mutex> guard{mtx};
        auto it = map.find(tid);
        if (map.end() != it) {
            return it->second;
        } else {
            throw wilton::support::exception(TRACEMSG("Invalid unknown TL entry to peek," + 
                    " tid: [" + tid + "], keys: [" + dump() + "]"));
        }
    }

    T remove() {
        auto tid = sl::support::to_string_any(std::this_thread::get_id());
        std::lock_guard<std::mutex> guard{mtx};
        auto it = map.find(tid);
        if (map.end() != it) {
            auto res = std::move(it->second);
            map.erase(it);
            return res;
        } else {
            throw wilton::support::exception(TRACEMSG("Invalid unknown TL entry to remove," + 
                    " tid: [" + tid + "], keys: [" + dump() + "]"));
        }
    }

    std::string dump() {
        std::lock_guard<std::mutex> guard{mtx};
        auto res = std::vector<sl::json::value>();
        for (const auto& en : map) {
            res.emplace_back(en.first);
        }
        return sl::json::value(std::move(res)).dumps();
    }
};

} // namespace
}

#endif /* WILTON_SUPPORT_TL_REGISTRY_HPP */


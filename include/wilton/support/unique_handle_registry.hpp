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
 * File:   unique_handle_registry.hpp
 * Author: alex
 *
 * Created on March 15, 2019, 10:23 AM
 */

#ifndef WILTON_SUPPORT_UNIQUE_HANDLE_REGISTRY_HPP
#define WILTON_SUPPORT_UNIQUE_HANDLE_REGISTRY_HPP

#include <cstdint>
#include <memory>
#include <mutex>
#include <map>

#include "staticlib/config.hpp"
#include "staticlib/support.hpp"

namespace wilton {
namespace support {

template<typename T>
class unique_handle_registry {
    std::map<int64_t, std::unique_ptr<T, std::function<void(T*)>>> registry;
    std::mutex mtx;
    std::function<void(T*)> deleter;

public:

    unique_handle_registry(std::function<void(T*)> deleter_fun) :
    deleter(std::move(deleter_fun)) { }

    unique_handle_registry(const unique_handle_registry&) = delete;

    unique_handle_registry& operator=(const unique_handle_registry&) = delete;

    ~unique_handle_registry() STATICLIB_NOEXCEPT {
// msvcr doesn't like that in JNI mode
#ifndef STATICLIB_WINDOWS
        std::lock_guard<std::mutex> lock{mtx};
#endif // STATICLIB_WINDOWS
        registry.clear();
    }

    int64_t put(T* ptr) {
        std::lock_guard<std::mutex> lock{mtx};
        auto handle = reinterpret_cast<int64_t> (ptr);
        auto sptr = std::unique_ptr<T, std::function<void(T*)>>(ptr, deleter);
        auto pair = registry.insert(std::make_pair(handle,  std::move(sptr)));
        return pair.second ? handle : 0;
    }

    T* remove(int64_t handle) {
        std::lock_guard<std::mutex> lock{mtx};
        auto it = registry.find(handle);
        if (registry.end() != it) {
            auto res = std::move(it->second);
            registry.erase(handle);
            return res.release();
        } else {
            return nullptr;
        }
    }
};

} // namespace
}

#endif /* WILTON_SUPPORT_UNIQUE_HANDLE_REGISTRY_HPP */


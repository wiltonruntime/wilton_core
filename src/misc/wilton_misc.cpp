/*
 * Copyright 2017, alex at staticlibs.net
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
 * File:   wilton_misc.cpp
 * Author: alex
 * 
 * Created on June 4, 2016, 7:22 PM
 */

#include "wilton/wilton.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "staticlib/config.hpp"
#include "staticlib/utils.hpp"

#include "wilton/support/alloc.hpp"

#include "call/wiltoncall_internal.hpp"

namespace { // anonymous

// initialized from wiltoncall_init
std::shared_ptr<std::mutex> shared_mutex() {
    static auto mutex = std::make_shared<std::mutex>();
    return mutex;
}

// initialized from wiltoncall_init
std::shared_ptr<std::vector<std::function<void(const std::string&)>>> shared_cleaners_registry() {
    static auto registry = std::make_shared<std::vector<std::function<void(const std::string&)>>>();
    return registry;
}

} // namespace

char* wilton_alloc(int size_bytes) /* noexcept */ {
    if (!sl::support::is_uint32_positive(size_bytes)) {
        return nullptr;
    }
    return reinterpret_cast<char*>(std::malloc(static_cast<size_t>(size_bytes)));
}

void wilton_free(char* buffer) /* noexcept */ {
    std::free(buffer);
}

char* wilton_config(char** conf_json_out, int* conf_json_len_out) /* noexcept */ {
    if (nullptr == conf_json_out) return wilton::support::alloc_copy(TRACEMSG("Null 'conf_json_out' parameter specified"));
    if (nullptr == conf_json_len_out) return wilton::support::alloc_copy(TRACEMSG("Null 'conf_json_len_out' parameter specified"));
    try {
        auto ptr = wilton::internal::shared_wiltoncall_config();
        auto buf = wilton::support::make_json_buffer(*ptr);
        *conf_json_out = buf.data();
        *conf_json_len_out = buf.size_int();
        return nullptr;
    } catch (const std::exception& e) {
        return wilton::support::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

char* wilton_clean_tls(const char* thread_id, int thread_id_len) {
    if (nullptr == thread_id) return wilton::support::alloc_copy(TRACEMSG("Null 'thread_id' parameter specified"));
    if (!sl::support::is_uint16_positive(thread_id_len)) return wilton::support::alloc_copy(TRACEMSG(
            "Invalid 'thread_id_len' parameter specified: [" + sl::support::to_string(thread_id_len) + "]"));
    try {
        uint16_t thread_id_len_u16 = static_cast<uint16_t> (thread_id_len);
        auto tid = std::string(thread_id, thread_id_len_u16);
        auto mx = shared_mutex();
        std::lock_guard<std::mutex> guard{*mx};
        auto reg = shared_cleaners_registry();
        for (auto& fun : *reg) {
            fun(tid);
        }
        return nullptr;
    } catch (const std::exception& e) {
        return wilton::support::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

char* wilton_register_tls_cleaner(void* cleaner_ctx, void (*cleaner_cb)
        (void* cleaner_ctx, const char* thread_id, int thread_id_len)) /* noexcept */ {
    if (nullptr == cleaner_cb) return wilton::support::alloc_copy(TRACEMSG("Null 'cleaner_cb' parameter specified"));
    try {
        auto fun = [cleaner_ctx, cleaner_cb](const std::string& tid) STATICLIB_NOEXCEPT {
            cleaner_cb(cleaner_ctx, tid.c_str(), static_cast<int>(tid.length()));
        };
        auto mx = shared_mutex();
        std::lock_guard<std::mutex> guard{*mx};
        auto reg = shared_cleaners_registry();
        reg->emplace_back(std::move(fun));
        return nullptr;
    } catch (const std::exception& e) {
        return wilton::support::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

namespace wilton {
namespace internal {

// local static init is not thread-safe in vs2013
void init_tls_cleaners_registry() {
    shared_mutex();
    shared_cleaners_registry();
}

} // namespace
}

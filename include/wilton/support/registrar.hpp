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
 * File:   registrar.hpp
 * Author: alex
 *
 * Created on June 12, 2017, 4:55 PM
 */

#ifndef WILTON_SUPPORT_REGISTRAR_HPP
#define WILTON_SUPPORT_REGISTRAR_HPP

#include <functional>
#include <memory>
#include <utility>

#include "wilton/wilton.h"
#include "wilton/wiltoncall.h"

#include "wilton/support/alloc.hpp"
#include "wilton/support/buffer.hpp"
#include "wilton/support/exception.hpp"

namespace wilton {
namespace support {

namespace detail_registrar {

using cb_type = char* (*)(
        void* call_ctx,
        const char* json_in,
        int json_in_len,
        char** json_out,
        int* json_out_len);

using fun_span_type = support::buffer(*)(sl::io::span<const char>);

inline char* cb_fun(void* call_ctx, const char* json_in, int json_in_len, char** json_out, int* json_out_len) {
    auto fun = reinterpret_cast<fun_span_type> (call_ctx);
    try {
        auto out = fun({json_in, json_in_len});
        if (!out.is_null()) {
            *json_out = out.data();
            *json_out_len = out.size_int();
        } else {
            *json_out = nullptr;
            *json_out_len = 0;
        }
        return nullptr;
    } catch (const std::exception& e) {
        return alloc_copy(TRACEMSG(e.what()));
    }
}

} // namespace

inline void register_wiltoncall(const std::string& name, detail_registrar::fun_span_type fun) {
    if (nullptr == fun) {
        throw exception(TRACEMSG("Registrar error, invalid empty function specified," +
                " name: [" + name + "]"));
    }
    auto err = wiltoncall_register(name.c_str(), static_cast<int> (name.length()), 
            reinterpret_cast<void*> (fun), detail_registrar::cb_fun);
    if (nullptr != err) {
        auto msg = TRACEMSG(err);
        wilton_free(err);
        throw exception(msg);
    }
}

} // namespace
}

#endif /* WILTON_SUPPORT_REGISTRAR_HPP */


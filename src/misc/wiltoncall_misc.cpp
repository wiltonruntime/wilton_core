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
 * File:   wiltoncall_misc.cpp
 * Author: alex
 *
 * Created on January 10, 2017, 12:34 PM
 */

#include <iostream>
#include <string>

#include "call/wiltoncall_internal.hpp"

#include "wilton/wilton.h"

namespace wilton {
namespace misc {

support::buffer get_wiltoncall_config(sl::io::span<const char>) {
    return support::make_json_buffer(*internal::shared_wiltoncall_config());
}

support::buffer stdin_readline(sl::io::span<const char>) {
    std::string res;
    std::getline(std::cin,res);
    return support::make_string_buffer(res);
}

support::buffer run_garbage_collector(sl::io::span<const char> data) {
    const std::string& default_engine = internal::shared_wiltoncall_config()
            ->getattr("defaultScriptEngine").as_string_nonempty_or_throw("defaultScriptEngine");
    auto callname = "rungc_" + default_engine;
    // call engine
    char* out = nullptr;
    int out_len = 0;
    auto err = wiltoncall(callname.c_str(), static_cast<int>(callname.length()),
            data.data(), data.size(),
            std::addressof(out), std::addressof(out_len));
    if (nullptr != err) {
        support::throw_wilton_error(err, TRACEMSG(err));
    }

    return support::make_array_buffer(out, out_len);
}

} // namespace
}

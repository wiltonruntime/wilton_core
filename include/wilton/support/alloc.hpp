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
 * File:   alloc_copy.hpp
 * Author: alex
 *
 * Created on June 12, 2017, 12:57 PM
 */

#ifndef WILTON_SUPPORT_ALLOC_COPY_HPP
#define WILTON_SUPPORT_ALLOC_COPY_HPP

#include <cstring>

#include "staticlib/io.hpp"
#include "staticlib/support.hpp"

#include "wilton/wilton.h"

namespace wilton {
namespace support {

inline sl::io::span<char> alloc_span(size_t size) {
    char* data = wilton_alloc(static_cast<int>(size + 1));
    std::memset(data, '\0', size + 1);
    return sl::io::make_span(data, size);
}

template<typename Buffer>
sl::io::span<char> alloc_copy_span(Buffer buf) {
    auto sink = sl::io::make_array_sink(wilton_alloc, wilton_free, buf.size());
    sink.write(buf);
    return sink.release();
}

inline char* alloc_copy(const std::string& str) STATICLIB_NOEXCEPT {
    try {
        return alloc_copy_span(str).data();
    } catch(...) {
        // bad alloc
        char* err = static_cast<char*> (wilton_alloc(2));
        err[0] = 'E';
        err[1] = '\0';
        return err;
    }
}

} // namespace
}

#endif /* WILTON_SUPPORT_ALLOC_COPY_HPP */


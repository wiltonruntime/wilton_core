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
 * File:   span_operations.hpp
 * Author: alex
 *
 * Created on June 12, 2017, 12:42 PM
 */

#ifndef WILTON_SUPPORT_SPAN_OPERATIONS_HPP
#define WILTON_SUPPORT_SPAN_OPERATIONS_HPP

#include <cstring>

#include "staticlib/io.hpp"
#include "staticlib/json.hpp"
#include "staticlib/support.hpp"

#include "wilton/wilton.h"

#include "wilton/support/alloc.hpp"

namespace wilton {
namespace support {

// allocated with wilton_alloc
using buffer = sl::io::span<char>;

inline buffer make_null_buffer() {
    return sl::io::span<char>(nullptr, 0);
}

inline buffer make_empty_buffer() {
    return alloc_span(0);
}

inline buffer make_const_span_buffer(sl::io::span<const char> span) {
    if (!span.is_null()) {
        return alloc_copy_span(span);
    } else {
        return make_null_buffer();
    }
}

inline buffer make_span_buffer(sl::io::span<char> span) {
    auto span_const = sl::io::make_span(const_cast<const char*>(span.data()), span.size());
    return make_const_span_buffer(span_const);
}

inline buffer make_array_buffer(const char* buf, int buf_len) {
    if (nullptr != buf) {
        auto span = sl::io::make_span(buf, buf_len);
        return make_const_span_buffer(std::move(span));
    } else {
        return make_null_buffer();
    }
}

inline buffer make_string_buffer(const std::string& st) {
    return alloc_copy_span(st);
}

inline buffer make_json_buffer(const sl::json::value& val) {
    auto sink = sl::io::make_array_sink(wilton_alloc, wilton_free);
    val.dump(sink);
    return sink.release();
}

template<typename Source>
buffer make_source_buffer(Source& src) {
    auto sink = sl::io::make_array_sink(wilton_alloc, wilton_free);
    sl::io::copy_all(src, sink);
    return sink.release();
}

template<typename Source>
buffer make_hex_buffer(Source& src) {
    auto dest = sl::io::make_array_sink(wilton_alloc, wilton_free);
    {
        auto sink = sl::io::make_hex_sink(dest);
        sl::io::copy_all(src, sink);
    }
    return dest.release();
}

inline buffer wrap_wilton_buffer(char* buf, int buf_len) {
    if (nullptr != buf) {
        return sl::io::make_span(buf, buf_len);
    } else {
        return make_null_buffer();
    }
}

} // namespace
}

#endif /* WILTON_SUPPORT_SPAN_OPERATIONS_HPP */


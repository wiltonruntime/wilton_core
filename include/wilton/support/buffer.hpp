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

#include "wilton/support/alloc_copy.hpp"

namespace wilton {
namespace support {

using buffer = sl::support::optional<sl::io::span<char>>;

inline buffer make_empty_buffer() {
    return sl::support::optional<sl::io::span<char>>();
}

inline buffer make_buffer(uint32_t size) {
    char* data = wilton_alloc(static_cast<int>(size + 1));
    std::memset(data, '\0', size + 1);
    return sl::support::make_optional(sl::io::make_span(data, size));
}

inline buffer make_const_span_buffer(sl::io::span<const char> span) {
    if (!span.empty()) {
        auto span_copied = alloc_copy_span(span);
        return sl::support::make_optional(std::move(span_copied));
    } else {
        return make_empty_buffer();
    }
}

inline buffer make_span_buffer(sl::io::span<char> span) {
    auto span_const = sl::io::make_span(const_cast<const char*>(span.data()), span.size());
    return make_const_span_buffer(span_const);
}

inline buffer make_array_buffer(const char* buf, int buf_len) {
    return make_const_span_buffer({buf, buf_len});
}

inline buffer make_string_buffer(const std::string& st) {
    auto span = alloc_copy_span(st);
    return sl::support::make_optional(std::move(span));
}

inline buffer make_json_buffer(const sl::json::value& val) {
    auto sink = sl::io::make_array_sink(wilton_alloc, wilton_free);
    val.dump(sink);
    return sl::support::make_optional(sink.release());
}

template<typename Source>
buffer make_source_buffer(Source& src) {
    auto sink = sl::io::make_array_sink(wilton_alloc, wilton_free);
    sl::io::copy_all(src, sink);
    return sl::support::make_optional(sink.release());
}

template<typename Source>
buffer make_hex_buffer(Source& src) {
    auto sink = sl::io::make_array_sink(wilton_alloc, wilton_free);
    sl::io::copy_to_hex(src, sink);
    return sl::support::make_optional(sink.release());
}

inline buffer wrap_wilton_buffer(char* buf, int buf_len) {
    if (nullptr != buf) {
        return sl::support::make_optional(sl::io::make_span(buf, buf_len));
    } else {
        return make_empty_buffer();
    }
}

} // namespace
}

#endif /* WILTON_SUPPORT_SPAN_OPERATIONS_HPP */


/* 
 * File:   span_operations.hpp
 * Author: alex
 *
 * Created on June 12, 2017, 12:42 PM
 */

#ifndef WILTON_SUPPORT_SPAN_OPERATIONS_HPP
#define	WILTON_SUPPORT_SPAN_OPERATIONS_HPP

#include "staticlib/io.hpp"
#include "staticlib/support.hpp"

#include "wilton/wilton.h"

#include "wilton/support/alloc_copy.hpp"

namespace wilton {
namespace support {

using buffer = sl::support::optional<sl::io::span<char>>;

inline buffer make_empty_buffer() {
    return sl::support::optional<sl::io::span<char>>();
}

inline buffer make_array_buffer(const char* buf, int buf_len) {
    if (nullptr != buf) {
        auto span_src = sl::io::make_span(buf, buf_len);
        auto span = alloc_copy_span(span_src);
        return sl::support::make_optional(std::move(span));
    } else {
        return make_empty_buffer();
    }
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

inline buffer wrap_wilton_buffer(char* buf, int buf_len) {
    if (nullptr != buf) {
        return sl::support::make_optional(sl::io::make_span(buf, buf_len));
    } else {
        return make_empty_buffer();
    }
}

} // namespace
}

#endif	/* WILTON_SUPPORT_SPAN_OPERATIONS_HPP */


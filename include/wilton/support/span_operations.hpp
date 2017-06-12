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

inline sl::support::optional<sl::io::span<char>> empty_span() {
    return sl::support::optional<sl::io::span<char>>();
}

inline sl::support::optional<sl::io::span<char>> into_span(const sl::json::value& val) {
    auto sink = sl::io::make_array_sink(wilton_alloc, wilton_free);
    val.dump(sink);
    return sl::support::make_optional(sink.release());
}

inline sl::support::optional<sl::io::span<char>> into_span(char* buf, int buf_len) {
    if (nullptr != buf) {
        return sl::support::make_optional(sl::io::make_span(buf, buf_len));
    } else {
        return empty_span();
    }
}

inline sl::support::optional<sl::io::span<char>> into_span(const std::string& st) {
    auto span = alloc_copy_span(st);
    return sl::support::make_optional(std::move(span));
}

} // namespace
}

#endif	/* WILTON_SUPPORT_SPAN_OPERATIONS_HPP */


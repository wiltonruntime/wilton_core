/* 
 * File:   alloc_copy.hpp
 * Author: alex
 *
 * Created on June 12, 2017, 12:57 PM
 */

#ifndef WILTON_SUPPORT_ALLOC_COPY_HPP
#define	WILTON_SUPPORT_ALLOC_COPY_HPP

#include "staticlib/io.hpp"
#include "staticlib/support.hpp"

#include "wilton/wilton.h"

namespace wilton {
namespace support {

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

#endif	/* WILTON_SUPPORT_ALLOC_COPY_HPP */


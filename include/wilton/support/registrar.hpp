/* 
 * File:   registrar.hpp
 * Author: alex
 *
 * Created on June 12, 2017, 4:55 PM
 */

#ifndef WILTON_SUPPORT_REGISTRAR_HPP
#define	WILTON_SUPPORT_REGISTRAR_HPP

#include <functional>
#include <memory>
#include <utility>

#include "wilton/wilton.h"
#include "wilton/wiltoncall.h"

#include "wilton/support/alloc_copy.hpp"
#include "wilton/support/buffer.hpp"
#include "wilton/support/wilton_support_exception.hpp"

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
        if (out) {
            *json_out = out.value().data();
            *json_out_len = static_cast<int> (out.value().size());
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
        throw wilton_support_exception(TRACEMSG("Registrar error, invalid empty function specified," +
                " name: [" + name + "]"));
    }
    auto err = wiltoncall_register(name.c_str(), static_cast<int> (name.length()), 
            reinterpret_cast<void*> (fun), detail_registrar::cb_fun);
    if (nullptr != err) {
        auto msg = TRACEMSG(err);
        wilton_free(err);
        throw wilton_support_exception(msg);
    }
}

} // namespace
}

#endif	/* WILTON_SUPPORT_REGISTRAR_HPP */


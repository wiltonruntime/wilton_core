/* 
 * File:   wiltoncall_load.cpp
 * Author: alex
 *
 * Created on July 17, 2017, 9:03 PM
 */

#include "call/wiltoncall_internal.hpp"


namespace wilton {
namespace load {

support::buffer load_module_resource(sl::io::span<const char> data) {
    char* out = nullptr;
    int out_len = 0;
    char* err = wilton_load_resource(data.data(), static_cast<int>(data.size()),
            std::addressof(out), std::addressof(out_len));
    if (nullptr != err) {
        support::throw_wilton_error(err, TRACEMSG(err));
    }
    return support::wrap_wilton_buffer(out, out_len);
}

support::buffer load_module_script(sl::io::span<const char> data) {
    char* out = nullptr;
    int out_len = 0;
    char* err = wilton_load_script(data.data(), static_cast<int>(data.size()),
            std::addressof(out), std::addressof(out_len));
    if (nullptr != err) {
        support::throw_wilton_error(err, TRACEMSG(err));
    }
    return support::wrap_wilton_buffer(out, out_len);
}

} // namespace
}

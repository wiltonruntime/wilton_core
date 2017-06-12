/* 
 * File:   wiltoncall_mustache.cpp
 * Author: alex
 *
 * Created on January 10, 2017, 5:53 PM
 */

#include "call/wiltoncall_internal.hpp"

#include "wilton/wilton.h"

namespace wilton {
namespace mustache {

sl::support::optional<sl::io::span<char>> mustache_render(sl::io::span<const char> data) {
    // parse json
    auto json = sl::json::load(data);
    auto rtemplate = std::ref(sl::utils::empty_string());
    std::string values = sl::utils::empty_string();
    for (const sl::json::field& fi : json.as_object()) {
        auto& name = fi.name();
        if ("template" == name) {
            rtemplate = fi.as_string_nonempty_or_throw(name);
        } else if ("values" == name) {
            values = fi.val().dumps();
        } else {
            throw common::wilton_internal_exception(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (rtemplate.get().empty()) throw common::wilton_internal_exception(TRACEMSG(
            "Required parameter 'template' not specified"));
    const std::string& templade = rtemplate.get();
    if (values.empty()) {
        values = "{}";
    }
    // call wilton
    char* out = nullptr;
    int out_len = 0;
    char* err = wilton_render_mustache(templade.c_str(), static_cast<int>(templade.length()),
            values.c_str(), static_cast<int>(values.length()), std::addressof(out), std::addressof(out_len));
    if (nullptr != err) common::throw_wilton_error(err, TRACEMSG(err));
    return support::into_span(out, out_len);
}

sl::support::optional<sl::io::span<char>> mustache_render_file(sl::io::span<const char> data) {
    // parse json
    auto json = sl::json::load(data);
    auto rfile = std::ref(sl::utils::empty_string());
    std::string values = sl::utils::empty_string();
    for (const sl::json::field& fi : json.as_object()) {
        auto& name = fi.name();
        if ("file" == name) {
            rfile = fi.as_string_nonempty_or_throw(name);
        } else if ("values" == name) {
            values = fi.val().dumps();
        } else {
            throw common::wilton_internal_exception(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (rfile.get().empty()) throw common::wilton_internal_exception(TRACEMSG(
            "Required parameter 'file' not specified"));
    const std::string& file = rfile.get();
    if (values.empty()) {
        values = "{}";
    }
    // call wilton
    char* out = nullptr;
    int out_len = 0;
    char* err = wilton_render_mustache_file(file.c_str(), static_cast<int>(file.length()),
            values.c_str(), static_cast<int>(values.length()), std::addressof(out), std::addressof(out_len));
    if (nullptr != err) common::throw_wilton_error(err, TRACEMSG(err));
    return support::into_span(out, out_len);
}

} // namespace
}

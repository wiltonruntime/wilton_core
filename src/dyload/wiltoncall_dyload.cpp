/* 
 * File:   wiltoncall_dyload.cpp
 * Author: alex
 *
 * Created on June 6, 2017, 7:26 PM
 */

#include <mutex>
#include <string>
#include <unordered_set>

#include "staticlib/config.hpp"
#include "staticlib/utils.hpp"

#include "wilton/wilton.h"

#include "call/wiltoncall_internal.hpp"

namespace wilton {
namespace dyload {

support::buffer dyload_shared_library(sl::io::span<const char> data) {
    // json parse
    auto json = sl::json::load(data);
    auto rname = std::ref(sl::utils::empty_string());
    auto rdirectory = std::ref(sl::utils::empty_string());
    for (const sl::json::field& fi : json.as_object()) {
        auto& name = fi.name();
        if ("name" == name) {
            rname = fi.as_string_nonempty_or_throw(name);
        } else if ("directory" == name) {
            rdirectory = fi.as_string_nonempty_or_throw(name);
        } else {
            throw support::exception(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (rname.get().empty()) throw support::exception(TRACEMSG(
            "Required parameter 'path' not specified"));
    const std::string& name = rname.get();
    const std::string& directory = rdirectory.get();
    // call wilton
    auto err = wilton_dyload(name.c_str(), static_cast<int>(name.length()),
            directory.c_str(), static_cast<int>(directory.length()));
    if (nullptr != err) {
        support::throw_wilton_error(err, TRACEMSG(err));
    }
    return support::make_empty_buffer();
}

} // namespace
}

/* 
 * File:   wiltoncall_misc.cpp
 * Author: alex
 *
 * Created on January 10, 2017, 12:34 PM
 */

#include <iostream>
#include <string>

#include "call/wiltoncall_internal.hpp"

#include "wilton/wilton.h"

namespace wilton {
namespace misc {

support::buffer process_spawn(sl::io::span<const char> data) {
    // json parse
    auto json = sl::json::load(data);
    auto rexecutable = std::ref(sl::utils::empty_string());
    auto args = std::vector<std::string>();
    auto routfile = std::ref(sl::utils::empty_string());
    bool await_exit = false;
    for (const sl::json::field& fi : json.as_object()) {
        auto& name = fi.name();
        if ("executable" == name) {
            rexecutable = fi.as_string_nonempty_or_throw(name);
        } else if ("args" == name) {
            for (const sl::json::value& el : fi.as_array_or_throw(name)) {
                args.emplace_back(el.as_string_nonempty_or_throw(name));
            }
        } else if ("outputFile" == name) {
            routfile = fi.as_string_nonempty_or_throw(name);
        } else if ("awaitExit" == name) {
            await_exit = fi.as_bool_or_throw(name);
        } else {
            throw support::exception(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (rexecutable.get().empty()) throw support::exception(TRACEMSG(
            "Required parameter 'executable' not specified"));
    if (routfile.get().empty()) throw support::exception(TRACEMSG(
            "Required parameter 'outFile' not specified"));    
    const std::string& executable = rexecutable.get();
    const std::string& outfile = routfile.get();
    // call utils
    int pid = 0;
    if (await_exit) {
        pid = sl::utils::exec_and_wait(executable, args, outfile);
    } else {
        pid = sl::utils::exec_async(executable, args, outfile);
    }
    return support::make_json_buffer(sl::json::value(pid));
}

support::buffer get_wiltoncall_config(sl::io::span<const char>) {
    return support::make_json_buffer(internal::static_wiltoncall_config());
}

support::buffer stdin_readline(sl::io::span<const char>) {
    std::string res;
    std::getline(std::cin,res);
    return support::make_string_buffer(res);
}

} // namespace
}

/* 
 * File:   wiltoncall_logging.cpp
 * Author: alex
 *
 * Created on January 10, 2017, 6:01 PM
 */

#include "call/wiltoncall_internal.hpp"

#include "wilton/wilton.h"

namespace wilton {
namespace logging {

sl::support::optional<sl::io::span<char>> logging_initialize(sl::io::span<const char> data) {
    char* err = wilton_logger_initialize(data.data(), static_cast<int>(data.size()));
    if (nullptr != err) common::throw_wilton_error(err, TRACEMSG(std::string(err)));
    return support::empty_span();
}

sl::support::optional<sl::io::span<char>> logging_log(sl::io::span<const char> data) {
    // json parse
    auto json = sl::json::load(data);
    auto rlevel = std::ref(sl::utils::empty_string());
    auto rlogger = std::ref(sl::utils::empty_string());
    auto rmessage = std::ref(sl::utils::empty_string());
    for (const sl::json::field& fi : json.as_object()) {
        auto& name = fi.name();
        if ("level" == name) {
            rlevel = fi.as_string_nonempty_or_throw(name);
        } else if ("logger" == name) {
            rlogger = fi.as_string_nonempty_or_throw(name);
        } else if ("message" == name) {
            rmessage = fi.as_string();
        } else {
            throw common::wilton_internal_exception(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (rlevel.get().empty()) throw common::wilton_internal_exception(TRACEMSG(
            "Required parameter 'level' not specified"));
    if (rlogger.get().empty()) throw common::wilton_internal_exception(TRACEMSG(
            "Required parameter 'logger' not specified"));
    const std::string& level = rlevel.get();
    const std::string& logger = rlogger.get();
    const std::string& message = rmessage.get();
    // call wilton
    char* err = wilton_logger_log(level.c_str(), static_cast<int>(level.length()),
            logger.c_str(), static_cast<int>(logger.length()),
            message.c_str(), static_cast<int>(message.length()));
    if (nullptr != err) common::throw_wilton_error(err, TRACEMSG(std::string(err)));
    return support::empty_span();
}

sl::support::optional<sl::io::span<char>> logging_is_level_enabled(sl::io::span<const char> data) {
    // parse json
    auto json = sl::json::load(data);
    auto rlevel = std::ref(sl::utils::empty_string());
    auto rlogger = std::ref(sl::utils::empty_string());
    for (const sl::json::field& fi : json.as_object()) {
        auto& name = fi.name();
        if ("level" == name) {
            rlevel = fi.as_string_nonempty_or_throw(name);
        } else if ("logger" == name) {
            rlogger = fi.as_string_nonempty_or_throw(name);
        } else {
            throw common::wilton_internal_exception(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (rlevel.get().empty()) throw common::wilton_internal_exception(TRACEMSG(
            "Required parameter 'level' not specified"));
    if (rlogger.get().empty()) throw common::wilton_internal_exception(TRACEMSG(
            "Required parameter 'logger' not specified"));
    const std::string& level = rlevel.get();
    const std::string& logger = rlogger.get();
    // call wilton
    int out;
    char* err = wilton_logger_is_level_enabled(logger.c_str(), static_cast<int>(logger.length()),
            level.c_str(), static_cast<int>(level.length()), std::addressof(out));
    if (nullptr != err) common::throw_wilton_error(err, TRACEMSG(std::string(err)));
    return support::into_span({
        { "enabled", out != 0 }
    });
}

sl::support::optional<sl::io::span<char>> logging_shutdown(sl::io::span<const char>) {
    // call wilton
    char* err = wilton_logger_shutdown();
    if (nullptr != err) common::throw_wilton_error(err, TRACEMSG(std::string(err)));
    return support::empty_span();
}

} // namespace
}

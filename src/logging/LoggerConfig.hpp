/* 
 * File:   LoggerConfig.hpp
 * Author: alex
 *
 * Created on May 10, 2016, 8:01 PM
 */

#ifndef WILTON_LOGGING_LOGGERCONFIG_HPP
#define	WILTON_LOGGING_LOGGERCONFIG_HPP

#include <string>

#include "staticlib/config.hpp"
#include "staticlib/serialization.hpp"

#include "common/WiltonInternalException.hpp"
#include "common/utils.hpp"

namespace wilton {
namespace logging {

class LoggerConfig {
public:
    std::string name = "";
    std::string level = "";

    LoggerConfig(const LoggerConfig&) = delete;

    LoggerConfig& operator=(const LoggerConfig&) = delete;

    LoggerConfig(LoggerConfig&& other) :
    name(std::move(other.name)),
    level(std::move(other.level)) { }

    LoggerConfig& operator=(LoggerConfig&& other) {
        this->name = std::move(other.name);
        this->level = std::move(other.level);
        return *this;
    }

    LoggerConfig() { }

    LoggerConfig(const staticlib::serialization::JsonValue& json) {
        namespace ss = staticlib::serialization;
        for (const ss::JsonField& fi : json.as_object()) {
            auto& fname = fi.name();
            if ("name" == fname) {
                this->name = common::get_json_string(fi, "logging.loggers.name");
            } else if ("level" == fname) {
                this->level = common::get_json_string(fi, "logging.loggers.level");
            } else {
                throw common::WiltonInternalException(TRACEMSG("Unknown 'logging.loggers' field: [" + fname + "]"));
            }
        }
        if (0 == name.length()) throw common::WiltonInternalException(TRACEMSG(
                "Invalid 'logging.loggers.name' field: []"));
        if (0 == level.length()) throw common::WiltonInternalException(TRACEMSG(
                "Invalid 'logging.loggers.level' field: []"));
    }

    staticlib::serialization::JsonValue to_json() const {
        return {
            {"name", name},
            {"level", level}
        };
    }
};

} // namepspace
}

#endif	/* WILTON_LOGGING_LOGGERCONFIG_HPP */


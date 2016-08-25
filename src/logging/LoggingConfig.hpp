/* 
 * File:   LoggingConfig.hpp
 * Author: alex
 *
 * Created on May 10, 2016, 5:12 PM
 */

#ifndef WILTON_LOGGING_LOGGINGCONFIG_HPP
#define	WILTON_LOGGING_LOGGINGCONFIG_HPP

#include <string>
#include <vector>

#include "staticlib/ranges.hpp"
#include "staticlib/serialization.hpp"

#include "common/WiltonInternalException.hpp"
#include "common/utils.hpp"
#include "logging/AppenderConfig.hpp"
#include "logging/LoggerConfig.hpp"

namespace wilton {
namespace logging {

class LoggingConfig {
public:
    std::vector<AppenderConfig> appenders;
    std::vector<LoggerConfig> loggers;

    LoggingConfig(const LoggingConfig&) = delete;

    LoggingConfig& operator=(const LoggingConfig&) = delete;

    LoggingConfig(LoggingConfig&& other) :
    appenders(std::move(other.appenders)),
    loggers(std::move(other.loggers)) { }

    LoggingConfig& operator=(LoggingConfig&& other) {
        this->appenders = std::move(other.appenders);
        this->loggers = std::move(other.loggers);
        return *this;
    }

    LoggingConfig() { }

    LoggingConfig(const staticlib::serialization::JsonValue& json) {
        namespace ss = staticlib::serialization;
        for (const ss::JsonField& fi : json.as_object()) {
            auto& name = fi.name();
            if ("appenders" == name) {
                for (const ss::JsonValue& ap : common::get_json_array(fi, "logging.appenders")) {
                    auto ja = AppenderConfig(ap);
                    appenders.emplace_back(std::move(ja));
                }
            } else if ("loggers" == name) {
                for (const ss::JsonValue& lo : common::get_json_array(fi, "logging.loggers")) {
                    auto jl = LoggerConfig(lo);
                    loggers.emplace_back(std::move(jl));
                }
            } else {
                throw common::WiltonInternalException(TRACEMSG("Unknown 'logging' field: [" + name + "]"));
            }
        }
    }

    staticlib::serialization::JsonValue to_json() const {
        namespace sr = staticlib::ranges;
        auto japps = sr::transform(sr::refwrap(appenders), [](const AppenderConfig& el) {
            return el.to_json();
        });
        auto jlogs = sr::transform(sr::refwrap(loggers), [](const LoggerConfig& el) {
            return el.to_json();
        });
        return {
            {"appenders", japps},
            {"loggers", jlogs}
        };
    }
};

} // namespace
}

#endif	/* WILTON_LOGGING_LOGGINGCONFIG_HPP */


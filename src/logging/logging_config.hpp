/* 
 * File:   logging_config.hpp
 * Author: alex
 *
 * Created on May 10, 2016, 5:12 PM
 */

#ifndef WILTON_LOGGING_LOGGING_CONFIG_HPP
#define	WILTON_LOGGING_LOGGING_CONFIG_HPP

#include <string>
#include <vector>

#include "staticlib/ranges.hpp"
#include "staticlib/serialization.hpp"

#include "common/wilton_internal_exception.hpp"
#include "common/utils.hpp"
#include "logging/appender_config.hpp"
#include "logging/logger_config.hpp"

namespace wilton {
namespace logging {

class logging_config {
public:
    std::vector<appender_config> appenders;
    std::vector<logger_config> loggers;

    logging_config(const logging_config&) = delete;

    logging_config& operator=(const logging_config&) = delete;

    logging_config(logging_config&& other) :
    appenders(std::move(other.appenders)),
    loggers(std::move(other.loggers)) { }

    logging_config& operator=(logging_config&& other) {
        this->appenders = std::move(other.appenders);
        this->loggers = std::move(other.loggers);
        return *this;
    }

    logging_config() { }

    logging_config(const staticlib::serialization::json_value& json) {
        namespace ss = staticlib::serialization;
        for (const ss::json_field& fi : json.as_object()) {
            auto& name = fi.name();
            if ("appenders" == name) {
                for (const ss::json_value& ap : fi.as_array_or_throw(name)) {
                    auto ja = appender_config(ap);
                    appenders.emplace_back(std::move(ja));
                }
            } else if ("loggers" == name) {
                for (const ss::json_value& lo : fi.as_array_or_throw(name)) {
                    auto jl = logger_config(lo);
                    loggers.emplace_back(std::move(jl));
                }
            } else {
                throw common::wilton_internal_exception(TRACEMSG("Unknown 'logging' field: [" + name + "]"));
            }
        }
    }

    staticlib::serialization::json_value to_json() const {
        namespace sr = staticlib::ranges;
        
        
        return {
            {"appenders", [this](){
                auto ra = sr::transform(appenders, [](const appender_config & el) {
                    return el.to_json();
                });
                return ra.to_vector();
            }()},
            {"loggers", [this](){
                auto ra = sr::transform(loggers, [](const logger_config & el) {
                    return el.to_json();
                });
                return ra.to_vector();
            }()}
        };
    }
};

} // namespace
}

#endif	/* WILTON_LOGGING_LOGGING_CONFIG_HPP */


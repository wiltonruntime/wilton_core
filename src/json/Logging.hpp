/* 
 * File:   Logging.hpp
 * Author: alex
 *
 * Created on May 10, 2016, 5:12 PM
 */

#ifndef WILTON_JSON_LOGGING_HPP
#define	WILTON_JSON_LOGGING_HPP

#include <string>
#include <vector>

#include "staticlib/ranges.hpp"
#include "staticlib/serialization.hpp"

#include "common/WiltonInternalException.hpp"
#include "json/Appender.hpp"
#include "json/Logger.hpp"

namespace wilton {
namespace json {

class Logging {
public:
    std::vector<Appender> appenders;
    std::vector<Logger> loggers;

    Logging(const Logging&) = delete;

    Logging& operator=(const Logging&) = delete;

    Logging(Logging&& other) :
    appenders(std::move(other.appenders)),
    loggers(std::move(other.loggers)) { }

    Logging& operator=(Logging&& other) {
        this->appenders = std::move(other.appenders);
        this->loggers = std::move(other.loggers);
        return *this;
    }

    Logging() { }

    Logging(const staticlib::serialization::JsonValue& json) {
        namespace ss = staticlib::serialization;
        for (const ss::JsonField& fi : json.get_object()) {
            auto& name = fi.get_name();
            if ("appenders" == name) {
                if (ss::JsonType::ARRAY != fi.get_type() || 0 == fi.get_array().size()) throw common::WiltonInternalException(TRACEMSG(std::string() +
                        "Invalid 'logging.appenders' field: [" + ss::dump_json_to_string(fi.get_value()) + "]"));
                for (const ss::JsonValue& ap : fi.get_array()) {
                    auto ja = json::Appender(ap);
                    appenders.emplace_back(std::move(ja));
                }
            } else if ("loggers" == name) {
                if (ss::JsonType::ARRAY != fi.get_type() || 0 == fi.get_array().size()) throw common::WiltonInternalException(TRACEMSG(std::string() +
                        "Invalid 'logging.loggers' field: [" + ss::dump_json_to_string(fi.get_value()) + "]"));
                for (const ss::JsonValue& lo : fi.get_array()) {
                    auto jl = json::Logger(lo);
                    loggers.emplace_back(std::move(jl));
                }
            } else {
                throw common::WiltonInternalException(TRACEMSG(std::string() +
                        "Unknown 'logging' field: [" + name + "]"));
            }
        }
    }

    staticlib::serialization::JsonValue to_json() const {
        namespace sr = staticlib::ranges;
        auto japps = sr::transform(sr::refwrap(appenders), [](const json::Appender& el) {
            return el.to_json();
        });
        auto jlogs = sr::transform(sr::refwrap(loggers), [](const json::Logger& el) {
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

#endif	/* WILTON_JSON_LOGGING_HPP */


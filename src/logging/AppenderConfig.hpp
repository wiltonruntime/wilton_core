/* 
 * File:   AppenderConfig.hpp
 * Author: alex
 *
 * Created on May 10, 2016, 7:55 PM
 */

#ifndef WILTON_LOGGING_APPENDERCONFIG_HPP
#define	WILTON_LOGGING_APPENDERCONFIG_HPP

#include <string>

#include "staticlib/config.hpp"
#include "staticlib/serialization.hpp"

#include "common/WiltonInternalException.hpp"
#include "common/utils.hpp"

namespace wilton {
namespace logging {

class AppenderConfig {
public:
    std::string appenderType = "NULL";
    std::string filePath = "";
    std::string layout = "%d{%Y-%m-%d %H:%M:%S,%q} [%-5p %-5.5t %-20.20c] %m%n";
    std::string thresholdLevel = "TRACE";

    AppenderConfig(const AppenderConfig&) = delete;

    AppenderConfig& operator=(const AppenderConfig&) = delete;

    AppenderConfig(AppenderConfig&& other) :
    appenderType(std::move(other.appenderType)),
    filePath(std::move(other.filePath)),
    layout(std::move(other.layout)),
    thresholdLevel(std::move(other.thresholdLevel)) { }

    AppenderConfig& operator=(AppenderConfig&& other) {
        this->appenderType = std::move(other.appenderType);
        this->filePath = std::move(other.filePath);
        this->layout = std::move(other.layout);
        this->thresholdLevel = std::move(other.thresholdLevel);
        return *this;
    }

    AppenderConfig() { }

    AppenderConfig(const staticlib::serialization::JsonValue& json) {
        namespace ss = staticlib::serialization;
        for (const ss::JsonField& fi : json.as_object()) {
            auto& name = fi.name();
            if ("appenderType" == name) {
                this->appenderType = common::get_json_string(fi, "logging.appenders.appenderType");
            } else if ("filePath" == name) {
                this->filePath = common::get_json_string(fi, "logging.appenders.filePath");
            } else if ("layout" == name) {
                this->layout = common::get_json_string(fi, "logging.appenders.layout");
            } else if ("thresholdLevel" == name) {
                this->thresholdLevel = common::get_json_string(fi, "logging.appenders.thresholdLevel");
            } else {
                throw common::WiltonInternalException(TRACEMSG("Unknown 'logging.appenders' field: [" + name + "]"));
            }
        }
        if (("FILE" == appenderType || "DAILY_ROLLING_FILE" == appenderType) &&
                filePath.empty()) throw common::WiltonInternalException(TRACEMSG(
                "Invalid 'logging.appenders.filePath' field: []"));
    }

    staticlib::serialization::JsonValue to_json() const {
        return {
            {"appenderType", appenderType},
            {"filePath", filePath},
            {"layout", layout},
            {"thresholdLevel", thresholdLevel}
        };
    }

};

} // namepspace
}

#endif	/* WILTON_LOGGING_APPENDERCONFIG_HPP */


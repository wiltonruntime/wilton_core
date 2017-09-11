/* 
 * File:   appender_config.hpp
 * Author: alex
 *
 * Created on May 10, 2016, 7:55 PM
 */

#ifndef WILTON_LOGGING_APPENDER_CONFIG_HPP
#define WILTON_LOGGING_APPENDER_CONFIG_HPP

#include <cstdint>
#include <string>

#include "staticlib/config.hpp"
#include "staticlib/json.hpp"

#include "common/wilton_internal_exception.hpp"
#include "common/utils.hpp"

namespace wilton {
namespace logging {

class appender_config {
public:
    std::string appenderType = "NULL";
    std::string filePath = "";
    std::string layout = "%d{%Y-%m-%d %H:%M:%S,%q} [%-5p %-5.5t %-20.20c] %m%n";
    std::string thresholdLevel = "TRACE";
    bool useLockFile = false;
    uint16_t maxBackupIndex = 16;

    appender_config(const appender_config&) = delete;

    appender_config& operator=(const appender_config&) = delete;

    appender_config(appender_config&& other) :
    appenderType(std::move(other.appenderType)),
    filePath(std::move(other.filePath)),
    layout(std::move(other.layout)),
    thresholdLevel(std::move(other.thresholdLevel)),
    useLockFile(other.useLockFile),
    maxBackupIndex(other.maxBackupIndex) { }

    appender_config& operator=(appender_config&& other) {
        this->appenderType = std::move(other.appenderType);
        this->filePath = std::move(other.filePath);
        this->layout = std::move(other.layout);
        this->thresholdLevel = std::move(other.thresholdLevel);
        this->useLockFile = other.useLockFile;
        this->maxBackupIndex = other.maxBackupIndex;
        return *this;
    }

    appender_config() { }

    appender_config(const sl::json::value& json) {
        for (const sl::json::field& fi : json.as_object()) {
            auto& name = fi.name();
            if ("appenderType" == name) {
                this->appenderType = fi.as_string_nonempty_or_throw(name);
            } else if ("filePath" == name) {
                this->filePath = fi.as_string_nonempty_or_throw(name);
            } else if ("layout" == name) {
                this->layout = fi.as_string_nonempty_or_throw(name);
            } else if ("thresholdLevel" == name) {
                this->thresholdLevel = fi.as_string_nonempty_or_throw(name);
            } else if ("useLockFile" == name) {
                this->useLockFile = fi.as_bool_or_throw(name);
            } else if ("maxBackupIndex" == name) {
                this->maxBackupIndex = fi.as_uint16_positive_or_throw(name);                
            } else {
                throw common::wilton_internal_exception(TRACEMSG("Unknown 'logging.appenders' field: [" + name + "]"));
            }
        }
        if (("FILE" == appenderType || "DAILY_ROLLING_FILE" == appenderType) &&
                filePath.empty()) throw common::wilton_internal_exception(TRACEMSG(
                "Invalid 'logging.appenders.filePath' field: []"));
    }

    sl::json::value to_json() const {
        return {
            {"appenderType", appenderType},
            {"filePath", filePath},
            {"layout", layout},
            {"thresholdLevel", thresholdLevel},
            {"useLockFile", useLockFile},
            {"maxBackupIndex", maxBackupIndex}
        };
    }

};

} // namepspace
}

#endif /* WILTON_LOGGING_APPENDER_CONFIG_HPP */


/* 
 * File:   Appender.hpp
 * Author: alex
 *
 * Created on May 10, 2016, 7:55 PM
 */

#ifndef WILTON_SERVERCONF_APPENDER_HPP
#define	WILTON_SERVERCONF_APPENDER_HPP

#include <string>

#include "staticlib/config.hpp"
#include "staticlib/serialization.hpp"

#include "common/WiltonInternalException.hpp"
#include "common/utils.hpp"

namespace wilton {
namespace serverconf {

class Appender {
public:
    std::string appenderType = "NULL";
    std::string filePath = "";
    std::string layout = "%d{%Y-%m-%d %H:%M:%S,%q} [%-5p %-5.5t %-20.20c] %m%n";
    std::string thresholdLevel = "TRACE";

    Appender(const Appender&) = delete;

    Appender& operator=(const Appender&) = delete;

    Appender(Appender&& other) :
    appenderType(std::move(other.appenderType)),
    filePath(std::move(other.filePath)),
    layout(std::move(other.layout)),
    thresholdLevel(std::move(other.thresholdLevel)) { }

    Appender& operator=(Appender&& other) {
        this->appenderType = std::move(other.appenderType);
        this->filePath = std::move(other.filePath);
        this->layout = std::move(other.layout);
        this->thresholdLevel = std::move(other.thresholdLevel);
        return *this;
    }

    Appender() { }

    Appender(const staticlib::serialization::JsonValue& json) {
        namespace ss = staticlib::serialization;
        for (const ss::JsonField& fi : json.get_object()) {
            auto& name = fi.get_name();
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

#endif	/* WILTON_SERVERCONF_APPENDER_HPP */


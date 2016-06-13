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
                if (0 == fi.get_string().length()) throw common::WiltonInternalException(TRACEMSG(std::string() +
                        "Invalid 'logging.appenders.appenderType' field: [" + ss::dump_json_to_string(fi.get_value()) + "]"));
                this->appenderType = fi.get_string();
            } else if ("filePath" == name) {
                if (0 == fi.get_string().length()) throw common::WiltonInternalException(TRACEMSG(std::string() +
                        "Invalid 'logging.appenders.filePath' field: [" + ss::dump_json_to_string(fi.get_value()) + "]"));
                this->filePath = fi.get_string();
            } else if ("layout" == name) {
                if (0 == fi.get_string().length()) throw common::WiltonInternalException(TRACEMSG(std::string() +
                        "Invalid 'logging.appenders.layout' field: [" + ss::dump_json_to_string(fi.get_value()) + "]"));
                this->layout = fi.get_string();
            } else if ("thresholdLevel" == name) {
                if (0 == fi.get_string().length()) throw common::WiltonInternalException(TRACEMSG(std::string() +
                        "Invalid 'logging.appenders.thresholdLevel' field: [" + ss::dump_json_to_string(fi.get_value()) + "]"));
                this->thresholdLevel = fi.get_string();
            } else {
                throw common::WiltonInternalException(TRACEMSG(std::string() +
                        "Unknown 'logging.appenders' field: [" + name + "]"));
            }
        }
        if (0 == appenderType.length()) throw common::WiltonInternalException(TRACEMSG(std::string() +
                "Invalid 'logging.appenders.appenderType' field: []"));
        if (("FILE" == appenderType || "DAILY_ROLLING_FILE" == appenderType) &&
                0 == filePath.length()) throw common::WiltonInternalException(TRACEMSG(std::string() +
                "Invalid 'logging.appenders.filePath' field: []"));
        if (0 == layout.length()) throw common::WiltonInternalException(TRACEMSG(std::string() +
                "Invalid 'logging.appenders.layout' field: []"));
        if (0 == thresholdLevel.length()) throw common::WiltonInternalException(TRACEMSG(std::string() +
                "Invalid 'logging.appenders.thresholdLevel' field: []"));
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


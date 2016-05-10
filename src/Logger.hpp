/* 
 * File:   Logger.hpp
 * Author: alex
 *
 * Created on May 10, 2016, 4:49 PM
 */

#ifndef WILTON_C_LOGGER_HPP
#define	WILTON_C_LOGGER_HPP

#include <string>

#include <log4cplus/logger.h>

#include "WiltonInternalException.hpp"

namespace wilton {
namespace c {

namespace { // anonymous

log4cplus::LogLevel get_log_level(const std::string& level_name) {
    if ("TRACE" == level_name) {
        return log4cplus::TRACE_LOG_LEVEL;
    } else if ("DEBUG" == level_name) {
        return log4cplus::DEBUG_LOG_LEVEL;
    } else if ("INFO" == level_name) {
        return log4cplus::INFO_LOG_LEVEL;
    } else if ("WARN" == level_name) {
        return log4cplus::WARN_LOG_LEVEL;
    } else if ("ERROR" == level_name) {
        return log4cplus::ERROR_LOG_LEVEL;
    } else if ("FATAL" == level_name) {
        return log4cplus::FATAL_LOG_LEVEL;
    } else {
        return log4cplus::INFO_LOG_LEVEL;
    }
}

} // namespace

class Logger {

public:
    static void log(const std::string& level_name, const std::string& logger_name, const std::string& message) {
        const auto& logger = log4cplus::Logger::getInstance(logger_name);
        log4cplus::LogLevel level = get_log_level(level_name);
        if (logger.isEnabledFor(level)) {
            logger.log(level, message);
        }
    }

};

} // namespace
}

#endif	/* WILTON_C_LOGGER_HPP */


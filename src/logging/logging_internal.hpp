/* 
 * File:   wilton_logging_internal.hpp
 * Author: alex
 *
 * Created on January 12, 2017, 3:17 PM
 */

#ifndef WILTON_LOGGING_LOGGING_INTERNAL_HPP
#define	WILTON_LOGGING_LOGGING_INTERNAL_HPP

// shouldn't be called before logging is initialized by app

inline void log_error(const std::string& logger, const std::string& message) {
    static std::string level = "ERROR";
    // call wilton
    wilton_logger_log(level.c_str(), level.length(), logger.c_str(), logger.length(),
            message.c_str(), message.length());
}

#endif	/* WILTON_LOGGING_LOGGING_INTERNAL_HPP */


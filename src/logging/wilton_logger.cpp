/* 
 * File:   wilton_logger.cpp
 * Author: alex
 * 
 * Created on June 2, 2016, 4:27 PM
 */

#include "logging/wilton_logger.hpp"

#include <atomic>

#include "log4cplus/logger.h"
#include "log4cplus/appender.h"
#include "log4cplus/consoleappender.h"
#include "log4cplus/nullappender.h"
#include "log4cplus/fileappender.h"

#include "staticlib/pimpl/pimpl_forward_macros.hpp"
#include "staticlib/utils.hpp"

#include "common/wilton_internal_exception.hpp"
#include "logging/appender_config.hpp"
#include "logging/logger_config.hpp"
#include "logging/logging_config.hpp"

namespace wilton {
namespace logging {

namespace { // anonymous

namespace su = staticlib::utils;

std::atomic_bool initialized{false};
std::atomic_bool shutted_down{false};
bool the_false{false};

log4cplus::LogLevel to_level(const std::string& level_name) {
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

class wilton_logger::impl : public staticlib::pimpl::pimpl_object::impl {
public:

    static void log(const std::string& level_name, const std::string& logger_name, const std::string& message) {
        const auto& logger = log4cplus::Logger::getInstance(logger_name);
        log4cplus::LogLevel level = to_level(level_name);
        if (logger.isEnabledFor(level)) {
            logger.log(level, message);
        }
    }

    static void apply_config(const logging_config& config) {
        if (initialized.compare_exchange_strong(the_false, true)) {
#ifndef STATICLIB_LINUX
            log4cplus::initialize();
#endif // STATICLIB_LINUX
            for (const auto& cf : config.appenders) {
                log4cplus::SharedAppenderPtr app{create_appender_ptr(cf)};
                app->setLayout(std::auto_ptr<log4cplus::Layout>(new log4cplus::PatternLayout(cf.layout)));
                app->setThreshold(to_level(cf.thresholdLevel));
                log4cplus::Logger::getRoot().addAppender(app);
            }
            for (const auto& cf : config.loggers) {
                auto lo = log4cplus::Logger::getInstance(cf.name);
                lo.setLogLevel(to_level(cf.level));
            }
        }
    }
    
    static bool is_enabled_for_level(const std::string& logger_name, const std::string& level_name) {
        const auto& logger = log4cplus::Logger::getInstance(logger_name);
        log4cplus::LogLevel level = to_level(level_name);
        return logger.isEnabledFor(level);
    }
    
    static void shutdown() {
        if (shutted_down.compare_exchange_strong(the_false, true)) {
            log4cplus::Logger::shutdown();
        }
    }

private:
    // todo: check invalid file behaviour
    static log4cplus::Appender* create_appender_ptr(const appender_config& conf) {
        if ("NULL" == conf.appenderType) {
            return new log4cplus::NullAppender();
        } else if ("CONSOLE" == conf.appenderType) {
            return new log4cplus::ConsoleAppender();
        } else if ("FILE" == conf.appenderType) {
            return new log4cplus::FileAppender(resolve_file_path(conf.filePath));
        } else if ("DAILY_ROLLING_FILE" == conf.appenderType) {
            return new log4cplus::DailyRollingFileAppender(resolve_file_path(conf.filePath));
        } else {
            throw common::wilton_internal_exception(TRACEMSG(
                    "Invalid 'logging.appender.appenderType': [" + conf.appenderType + "]"));
        }
    }
    
    static std::string resolve_file_path(const std::string& path) {
        if ((path.length() > 0 && '/' == path[0]) || 
                (path.length() > 1 && ':' == path[1])) {
            return path;
        }
        std::string execpath = su::current_executable_path();
        std::string dirpath = su::strip_filename(execpath);
        return dirpath + path;
    }

};
PIMPL_FORWARD_METHOD_STATIC(wilton_logger, void, log, (const std::string&)(const std::string&)(const std::string&), (), common::wilton_internal_exception)
PIMPL_FORWARD_METHOD_STATIC(wilton_logger, void, apply_config, (const logging_config&), (), common::wilton_internal_exception)
PIMPL_FORWARD_METHOD_STATIC(wilton_logger, bool, is_enabled_for_level, (const std::string&)(const std::string&), (), common::wilton_internal_exception)
PIMPL_FORWARD_METHOD_STATIC(wilton_logger, void, shutdown, (), (), common::wilton_internal_exception)

} // namespace
}


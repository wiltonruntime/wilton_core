/*
 * Copyright 2017, alex at staticlibs.net
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* 
 * File:   logging.hpp
 * Author: alex
 *
 * Created on October 15, 2017, 11:34 PM
 */

#ifndef WILTON_SUPPORT_LOGGING_HPP
#define WILTON_SUPPORT_LOGGING_HPP

#include "wilton/wilton_logging.h"

#include <string>

namespace wilton {
namespace support {

inline void log(const std::string& logger, const std::string& level, const std::string& message) {
    int out = 0;
    char* err_level = wilton_logger_is_level_enabled(logger.c_str(), static_cast<int>(logger.length()),
            level.c_str(), static_cast<int>(level.length()), std::addressof(out));
    if (nullptr != err_level) {
        wilton_free(err_level);
        return;
    }
    if(out != 0) {
        auto err_log = wilton_logger_log(level.c_str(), static_cast<int>(level.length()), 
                logger.c_str(), static_cast<int>(logger.length()),
                message.c_str(), static_cast<int>(message.length()));
        if (nullptr != err_log) {
            wilton_free(err_log);
        }
    }
}

inline void log_debug(const std::string& logger, const std::string& message) {
    log(logger, "DEBUG", message);
}

inline void log_info(const std::string& logger, const std::string& message) {
    log(logger, "INFO", message);
}

inline void log_warn(const std::string& logger, const std::string& message) {
    log(logger, "WARN", message);
}

inline void log_error(const std::string& logger, const std::string& message) {
    log(logger, "ERROR", message);
}

} // namespace
}

#endif /* WILTON_SUPPORT_LOGGING_HPP */


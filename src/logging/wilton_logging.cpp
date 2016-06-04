/* 
 * File:   wilton_logging.cpp
 * Author: alex
 * 
 * Created on June 4, 2016, 7:24 PM
 */

#include "wilton/wilton.h"

#include <cstdint>

#include "staticlib/config.hpp"
#include "staticlib/utils.hpp"

#include "logging/WiltonLogger.hpp"

namespace { // anonymous

namespace sc = staticlib::config;
namespace su = staticlib::utils;
namespace wl = wilton::logging;

} // namespace

// todo: fixme message copy
char* wilton_log(
        const char* level_name,
        int level_name_len,
        const char* logger_name,
        int logger_name_len,
        const char* message,
        int message_len) {
    if (nullptr == level_name) return su::alloc_copy(TRACEMSG(std::string() +
            "Null 'level_name' parameter specified"));
    if (level_name_len <= 0 ||
            static_cast<int64_t> (level_name_len) > std::numeric_limits<uint32_t>::max()) return su::alloc_copy(TRACEMSG(std::string() +
            "Invalid 'level_name_len' parameter specified: [" + sc::to_string(level_name_len) + "]"));
    if (nullptr == logger_name) return su::alloc_copy(TRACEMSG(std::string() +
            "Null 'logger_name' parameter specified"));
    if (logger_name_len <= 0 ||
            static_cast<int64_t> (logger_name_len) > std::numeric_limits<uint32_t>::max()) return su::alloc_copy(TRACEMSG(std::string() +
            "Invalid 'logger_name_len' parameter specified: [" + sc::to_string(logger_name_len) + "]"));
    if (nullptr == message) return su::alloc_copy(TRACEMSG(std::string() +
            "Null 'message' parameter specified"));
    if (message_len <= 0 ||
            static_cast<int64_t> (message_len) > std::numeric_limits<uint32_t>::max()) return su::alloc_copy(TRACEMSG(std::string() +
            "Invalid 'message_len' parameter specified: [" + sc::to_string(message_len) + "]"));
    try {
        uint32_t level_name_len_u32 = static_cast<uint32_t> (level_name_len);
        std::string level_name_str{level_name, level_name_len_u32};
        uint32_t logger_name_len_u32 = static_cast<uint32_t> (logger_name_len);
        std::string logger_name_str{logger_name, logger_name_len_u32};
        uint32_t message_len_u32 = static_cast<uint32_t> (message_len);
        std::string message_str{message, message_len_u32};
        wl::WiltonLogger::log(level_name_str, logger_name_str, message_str);
        return nullptr;
    } catch (const std::exception& e) {
        return su::alloc_copy(TRACEMSG(std::string() + e.what() + "\nException raised"));
    }
}

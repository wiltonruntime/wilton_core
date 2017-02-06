/* 
 * File:   wilton_logging.cpp
 * Author: alex
 * 
 * Created on June 4, 2016, 7:24 PM
 */

#include "wilton/wilton.h"

#include <cstdint>

#include "staticlib/config.hpp"
#include "staticlib/serialization.hpp"
#include "staticlib/utils.hpp"

#include "logging/WiltonLogger.hpp"
#include "logging/LoggingConfig.hpp"

namespace { // anonymous

namespace sc = staticlib::config;
namespace ss = staticlib::serialization;
namespace su = staticlib::utils;
namespace wl = wilton::logging;

} // namespace

char* wilton_logger_initialize(
        const char* conf_json,
        int conf_json_len) {
    if (nullptr == conf_json) return su::alloc_copy(TRACEMSG("Null 'conf_json' parameter specified"));
    if (!sc::is_uint32_positive(conf_json_len)) return su::alloc_copy(TRACEMSG(
            "Invalid 'conf_json_len' parameter specified: [" + sc::to_string(conf_json_len) + "]"));
    try {
        uint32_t conf_json_len_u32 = static_cast<uint32_t> (conf_json_len);
        std::string conf_json_str{conf_json, conf_json_len_u32};
        ss::JsonValue conf = ss::load_json_from_string(conf_json_str);
        wl::LoggingConfig lc{conf};
        wl::WiltonLogger::apply_config(std::move(lc));
        return nullptr;
    } catch (const std::exception& e) {
        return su::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

// todo: fixme message copy
char* wilton_logger_log(
        const char* level_name,
        int level_name_len,
        const char* logger_name,
        int logger_name_len,
        const char* message,
        int message_len) {
    if (nullptr == level_name) return su::alloc_copy(TRACEMSG("Null 'level_name' parameter specified"));
    if (!sc::is_uint16_positive(level_name_len)) return su::alloc_copy(TRACEMSG(
            "Invalid 'level_name_len' parameter specified: [" + sc::to_string(level_name_len) + "]"));
    if (nullptr == logger_name) return su::alloc_copy(TRACEMSG("Null 'logger_name' parameter specified"));
    if (!sc::is_uint16_positive(logger_name_len)) return su::alloc_copy(TRACEMSG(
            "Invalid 'logger_name_len' parameter specified: [" + sc::to_string(logger_name_len) + "]"));
    if (nullptr == message) return su::alloc_copy(TRACEMSG("Null 'message' parameter specified"));
    if (!sc::is_uint32(message_len)) return su::alloc_copy(TRACEMSG(
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
        return su::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

char* wilton_logger_is_level_enabled(
        const char* logger_name,
        int logger_name_len,
        const char* level_name,
        int level_name_len,
        int* res_out
        ) {
    if (nullptr == logger_name) return su::alloc_copy(TRACEMSG("Null 'logger_name' parameter specified"));
    if (!sc::is_uint16_positive(logger_name_len)) return su::alloc_copy(TRACEMSG(
            "Invalid 'logger_name_len' parameter specified: [" + sc::to_string(logger_name_len) + "]"));
    if (nullptr == level_name) return su::alloc_copy(TRACEMSG("Null 'level_name' parameter specified"));
    if (!sc::is_uint16_positive(level_name_len)) return su::alloc_copy(TRACEMSG(
            "Invalid 'level_name_len' parameter specified: [" + sc::to_string(level_name_len) + "]"));
    if (nullptr == res_out) return su::alloc_copy(TRACEMSG("Null 'res_out' parameter specified"));
    try {
        uint32_t logger_name_len_u32 = static_cast<uint32_t> (logger_name_len);
        std::string logger_name_str{logger_name, logger_name_len_u32};
        uint32_t level_name_len_u32 = static_cast<uint32_t> (level_name_len);
        std::string level_name_str{level_name, level_name_len_u32};
        bool res = wl::WiltonLogger::is_enabled_for_level(logger_name_str, level_name_str);
        if (res) {
            *res_out = 1;
        } else {
            *res_out = 0;
        }
        return nullptr;
    } catch (const std::exception& e) {
        return su::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

char* wilton_logger_shutdown() {
    try {
        wl::WiltonLogger::shutdown();
        return nullptr;
    } catch (const std::exception& e) {
        return su::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

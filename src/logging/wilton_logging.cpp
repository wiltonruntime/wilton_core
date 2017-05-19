/* 
 * File:   wilton_logging.cpp
 * Author: alex
 * 
 * Created on June 4, 2016, 7:24 PM
 */

#include "wilton/wilton.h"

#include <cstdint>

#include "staticlib/config.hpp"
#include "staticlib/json.hpp"
#include "staticlib/utils.hpp"

#include "logging/wilton_logger.hpp"
#include "logging/logging_config.hpp"

char* wilton_logger_initialize(
        const char* conf_json,
        int conf_json_len) {
    if (nullptr == conf_json) return sl::utils::alloc_copy(TRACEMSG("Null 'conf_json' parameter specified"));
    if (!sl::support::is_uint32_positive(conf_json_len)) return sl::utils::alloc_copy(TRACEMSG(
            "Invalid 'conf_json_len' parameter specified: [" + sl::support::to_string(conf_json_len) + "]"));
    try {
        uint32_t conf_json_len_u32 = static_cast<uint32_t> (conf_json_len);
        std::string conf_json_str{conf_json, conf_json_len_u32};
        sl::json::value conf = sl::json::loads(conf_json_str);
        wilton::logging::logging_config lc{conf};
        wilton::logging::wilton_logger::apply_config(std::move(lc));
        return nullptr;
    } catch (const std::exception& e) {
        return sl::utils::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
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
    if (nullptr == level_name) return sl::utils::alloc_copy(TRACEMSG("Null 'level_name' parameter specified"));
    if (!sl::support::is_uint16_positive(level_name_len)) return sl::utils::alloc_copy(TRACEMSG(
            "Invalid 'level_name_len' parameter specified: [" + sl::support::to_string(level_name_len) + "]"));
    if (nullptr == logger_name) return sl::utils::alloc_copy(TRACEMSG("Null 'logger_name' parameter specified"));
    if (!sl::support::is_uint16_positive(logger_name_len)) return sl::utils::alloc_copy(TRACEMSG(
            "Invalid 'logger_name_len' parameter specified: [" + sl::support::to_string(logger_name_len) + "]"));
    if (nullptr == message) return sl::utils::alloc_copy(TRACEMSG("Null 'message' parameter specified"));
    if (!sl::support::is_uint32(message_len)) return sl::utils::alloc_copy(TRACEMSG(
            "Invalid 'message_len' parameter specified: [" + sl::support::to_string(message_len) + "]"));
    try {
        uint32_t level_name_len_u32 = static_cast<uint32_t> (level_name_len);
        std::string level_name_str{level_name, level_name_len_u32};
        uint32_t logger_name_len_u32 = static_cast<uint32_t> (logger_name_len);
        std::string logger_name_str{logger_name, logger_name_len_u32};
        uint32_t message_len_u32 = static_cast<uint32_t> (message_len);
        std::string message_str{message, message_len_u32};
        wilton::logging::wilton_logger::log(level_name_str, logger_name_str, message_str);
        return nullptr;
    } catch (const std::exception& e) {
        return sl::utils::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

char* wilton_logger_is_level_enabled(
        const char* logger_name,
        int logger_name_len,
        const char* level_name,
        int level_name_len,
        int* res_out
        ) {
    if (nullptr == logger_name) return sl::utils::alloc_copy(TRACEMSG("Null 'logger_name' parameter specified"));
    if (!sl::support::is_uint16_positive(logger_name_len)) return sl::utils::alloc_copy(TRACEMSG(
            "Invalid 'logger_name_len' parameter specified: [" + sl::support::to_string(logger_name_len) + "]"));
    if (nullptr == level_name) return sl::utils::alloc_copy(TRACEMSG("Null 'level_name' parameter specified"));
    if (!sl::support::is_uint16_positive(level_name_len)) return sl::utils::alloc_copy(TRACEMSG(
            "Invalid 'level_name_len' parameter specified: [" + sl::support::to_string(level_name_len) + "]"));
    if (nullptr == res_out) return sl::utils::alloc_copy(TRACEMSG("Null 'res_out' parameter specified"));
    try {
        uint32_t logger_name_len_u32 = static_cast<uint32_t> (logger_name_len);
        std::string logger_name_str{logger_name, logger_name_len_u32};
        uint32_t level_name_len_u32 = static_cast<uint32_t> (level_name_len);
        std::string level_name_str{level_name, level_name_len_u32};
        bool res = wilton::logging::wilton_logger::is_enabled_for_level(logger_name_str, level_name_str);
        if (res) {
            *res_out = 1;
        } else {
            *res_out = 0;
        }
        return nullptr;
    } catch (const std::exception& e) {
        return sl::utils::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

char* wilton_logger_shutdown() {
    try {
        wilton::logging::wilton_logger::shutdown();
        return nullptr;
    } catch (const std::exception& e) {
        return sl::utils::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

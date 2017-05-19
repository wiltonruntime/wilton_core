/* 
 * File:   wilton_mustache.cpp
 * Author: alex
 * 
 * Created on June 4, 2016, 7:44 PM
 */

#include "wilton/wilton.h"

#include <cstdint>
#include <array>

#include "staticlib/config.hpp"
#include "staticlib/io.hpp"
#include "staticlib/mustache.hpp"
#include "staticlib/json.hpp"
#include "staticlib/utils.hpp"

// todo: stream json
char* wilton_render_mustache /* noexcept */ (
        const char* template_text,
        int template_text_len,
        const char* values_json,
        int values_json_len,
        char** output_text_out,
        int* output_text_len_out) /* noexcept */ {
    if (nullptr == template_text) return sl::utils::alloc_copy(TRACEMSG("Null 'template_text' parameter specified"));
    if (!sl::support::is_uint32(template_text_len)) return sl::utils::alloc_copy(TRACEMSG(
            "Invalid 'template_text_len' parameter specified: [" + sl::support::to_string(template_text_len) + "]"));
    if (nullptr == values_json) return sl::utils::alloc_copy(TRACEMSG("Null 'values_json' parameter specified"));
    if (!sl::support::is_uint32_positive(values_json_len)) return sl::utils::alloc_copy(TRACEMSG(
            "Invalid 'values_json_len' parameter specified: [" + sl::support::to_string(values_json_len) + "]"));
    if (nullptr == output_text_out) return sl::utils::alloc_copy(TRACEMSG("Null 'output_text_out' parameter specified"));
    if (nullptr == output_text_len_out) return sl::utils::alloc_copy(TRACEMSG("Null 'output_text_len_out' parameter specified"));
    try {
        uint32_t template_text_len_u32 = static_cast<uint32_t> (template_text_len);
        std::string template_text_str{template_text, template_text_len_u32};
        uint32_t values_json_len_u32 = static_cast<uint32_t> (values_json_len);
        std::string values_json_str{values_json, values_json_len_u32};
        sl::json::value json = sl::json::loads(values_json_str);
        const std::string res = sl::mustache::render_string(template_text_str, json);
        *output_text_out = sl::utils::alloc_copy(res);
        *output_text_len_out = static_cast<int>(res.length());
        return nullptr;
    } catch (const std::exception& e) {
        return sl::utils::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

// todo: copies
WILTON_EXPORT char* wilton_render_mustache_file /* noexcept */ (
        const char* template_file_path,
        int template_file_path_len,
        const char* values_json,
        int values_json_len,
        char** output_text_out,
        int* output_text_len_out) {
    if (nullptr == template_file_path) return sl::utils::alloc_copy(TRACEMSG("Null 'template_file_path' parameter specified"));
    if (!sl::support::is_uint16(template_file_path_len)) return sl::utils::alloc_copy(TRACEMSG(
            "Invalid 'template_file_path_len' parameter specified: [" + sl::support::to_string(template_file_path_len) + "]"));
    if (nullptr == values_json) return sl::utils::alloc_copy(TRACEMSG("Null 'values_json' parameter specified"));
    if (!sl::support::is_uint32_positive(values_json_len)) return sl::utils::alloc_copy(TRACEMSG(
            "Invalid 'values_json_len' parameter specified: [" + sl::support::to_string(values_json_len) + "]"));
    if (nullptr == output_text_out) return sl::utils::alloc_copy(TRACEMSG("Null 'output_text_out' parameter specified"));
    if (nullptr == output_text_len_out) return sl::utils::alloc_copy(TRACEMSG("Null 'output_text_len_out' parameter specified"));
    try {
        uint16_t template_file_path_len_u16 = static_cast<uint16_t> (template_file_path_len);
        std::string template_file_path_str{template_file_path, template_file_path_len_u16};
        uint32_t values_json_len_u32 = static_cast<uint32_t> (values_json_len);
        std::string values_json_str{values_json, values_json_len_u32};
        sl::json::value json = sl::json::loads(values_json_str);
        auto mp = sl::mustache::source(template_file_path_str, std::move(json));
        auto sink = sl::io::string_sink();
        sl::io::copy_all(mp, sink);
        *output_text_out = sl::utils::alloc_copy(sink.get_string());
        *output_text_len_out = static_cast<int>(sink.get_string().length());
        return nullptr;
    } catch (const std::exception& e) {
        return sl::utils::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

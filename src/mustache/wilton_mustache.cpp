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
#include "staticlib/serialization.hpp"
#include "staticlib/utils.hpp"

namespace { // anonymous

namespace sc = staticlib::config;
namespace si = staticlib::io;
namespace sm = staticlib::mustache;
namespace ss = staticlib::serialization;
namespace su = staticlib::utils;

} // namespace

// todo: stream json
char* wilton_render_mustache /* noexcept */ (
        const char* template_text,
        int template_text_len,
        const char* values_json,
        int values_json_len,
        char** output_text_out,
        int* output_text_len_out) /* noexcept */ {
    if (nullptr == template_text) return su::alloc_copy(TRACEMSG("Null 'template_text' parameter specified"));
    if (!su::is_uint32(template_text_len)) return su::alloc_copy(TRACEMSG(
            "Invalid 'template_text_len' parameter specified: [" + sc::to_string(template_text_len) + "]"));
    if (nullptr == values_json) return su::alloc_copy(TRACEMSG("Null 'values_json' parameter specified"));
    if (!su::is_positive_uint32(values_json_len)) return su::alloc_copy(TRACEMSG(
            "Invalid 'values_json_len' parameter specified: [" + sc::to_string(values_json_len) + "]"));
    if (nullptr == output_text_out) return su::alloc_copy(TRACEMSG("Null 'output_text_out' parameter specified"));
    if (nullptr == output_text_len_out) return su::alloc_copy(TRACEMSG("Null 'output_text_len_out' parameter specified"));
    try {
        uint32_t template_text_len_u32 = static_cast<uint32_t> (template_text_len);
        std::string template_text_str{template_text, template_text_len_u32};
        uint32_t values_json_len_u32 = static_cast<uint32_t> (values_json_len);
        std::string values_json_str{values_json, values_json_len_u32};
        ss::JsonValue json = ss::load_json_from_string(values_json_str);
        const std::string res = sm::render_string(template_text_str, json);
        *output_text_out = su::alloc_copy(res);
        *output_text_len_out = res.length();
        return nullptr;
    } catch (const std::exception& e) {
        return su::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
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
    if (nullptr == template_file_path) return su::alloc_copy(TRACEMSG("Null 'template_file_path' parameter specified"));
    if (!su::is_uint16(template_file_path_len)) return su::alloc_copy(TRACEMSG(
            "Invalid 'template_file_path_len' parameter specified: [" + sc::to_string(template_file_path_len) + "]"));
    if (nullptr == values_json) return su::alloc_copy(TRACEMSG("Null 'values_json' parameter specified"));
    if (!su::is_positive_uint32(values_json_len)) return su::alloc_copy(TRACEMSG(
            "Invalid 'values_json_len' parameter specified: [" + sc::to_string(values_json_len) + "]"));
    if (nullptr == output_text_out) return su::alloc_copy(TRACEMSG("Null 'output_text_out' parameter specified"));
    if (nullptr == output_text_len_out) return su::alloc_copy(TRACEMSG("Null 'output_text_len_out' parameter specified"));
    try {
        uint16_t template_file_path_len_u16 = static_cast<uint16_t> (template_file_path_len);
        std::string template_file_path_str{template_file_path, template_file_path_len_u16};
        uint32_t values_json_len_u32 = static_cast<uint32_t> (values_json_len);
        std::string values_json_str{values_json, values_json_len_u32};
        ss::JsonValue json = ss::load_json_from_string(values_json_str);
        auto mp = sm::MustacheSource(template_file_path_str, std::move(json));
        auto sink = si::string_sink();
        std::array<char, 4096> buf;
        si::copy_all(mp, sink, buf.data(), buf.size());
        *output_text_out = su::alloc_copy(sink.get_string());
        *output_text_len_out = sink.get_string().length();
        return nullptr;
    } catch (const std::exception& e) {
        return su::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

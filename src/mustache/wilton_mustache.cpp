/* 
 * File:   wilton_mustache.cpp
 * Author: alex
 * 
 * Created on June 4, 2016, 7:44 PM
 */

#include "wilton/wilton.h"

#include <cstdint>

#include "staticlib/config.hpp"
#include "staticlib/serialization.hpp"
#include "staticlib/utils.hpp"

#include "mustache/MustacheProcessor.hpp"

namespace { // anonymous

namespace sc = staticlib::config;
namespace ss = staticlib::serialization;
namespace su = staticlib::utils;
namespace wm = wilton::mustache;

} // namespace

// todo: stream json
char* wilton_render_mustache(
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
        const std::string res = wm::MustacheProcessor::render_string(template_text_str, json);
        *output_text_out = su::alloc_copy(res);
        *output_text_len_out = res.size();
        return nullptr;
    } catch (const std::exception& e) {
        return su::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

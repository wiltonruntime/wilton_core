/* 
 * File:   utils.cpp
 * Author: alex
 * 
 * Created on June 14, 2016, 11:45 AM
 */

#include "common/utils.hpp"

#include "staticlib/utils.hpp"

#include "wilton/wilton.h"

#include "common/WiltonInternalException.hpp"

namespace wilton {
namespace common {

namespace { // anonymous

namespace ss = staticlib::serialization;
namespace su = staticlib::utils;

} //namespace

const std::string& empty_string() {
    static std::string empty{""};
    return empty;
}

const ss::JsonValue& empty_json() {
    static ss::JsonValue empty;
    return empty;
}

void throw_wilton_error(char* err, const std::string& msg) {
    wilton_free(err);
    throw WiltonInternalException(msg);
}

std::string wrap_wilton_output(char* out, int out_len) {
    std::string res{out, static_cast<std::string::size_type> (out_len)};
    wilton_free(out);
    return res;
}

const std::string& get_json_string(const ss::JsonField& field) {
    if (ss::JsonType::STRING != field.type() || field.as_string().empty()) {
        throw common::WiltonInternalException(TRACEMSG("Invalid '" + field.name() + "' field,"
            " type: [" + ss::stringify_json_type(field.type()) + "]," +
            " value: [" + ss::dump_json_to_string(field.value()) + "]"));
    }
    return field.as_string();
}

int64_t get_json_int64(const ss::JsonField& field) {
    if (ss::JsonType::INTEGER != field.type()) {
        throw common::WiltonInternalException(TRACEMSG("Invalid '" + field.name() + "' field,"
                " type: [" + ss::stringify_json_type(field.type()) + "]," +
                " value: [" + ss::dump_json_to_string(field.value()) + "]"));
    }
    return field.as_int64();
}

uint32_t get_json_uint32(const staticlib::serialization::JsonField& field) {
    if (ss::JsonType::INTEGER != field.type() || !su::is_uint32(field.as_int64())) {
        throw common::WiltonInternalException(TRACEMSG("Invalid '" + field.name() + "' field,"
                " type: [" + ss::stringify_json_type(field.type()) + "]," +
                " value: [" + ss::dump_json_to_string(field.value()) + "]"));
    }
    return field.as_uint32();
}

uint16_t get_json_uint16(const staticlib::serialization::JsonField& field) {
    if (ss::JsonType::INTEGER != field.type() || !su::is_uint16(field.as_int64())) {
        throw common::WiltonInternalException(TRACEMSG("Invalid '" + field.name() + "' field,"
                " type: [" + ss::stringify_json_type(field.type()) + "]," +
                " value: [" + ss::dump_json_to_string(field.value()) + "]"));
    }
    return field.as_uint16();
}

bool get_json_bool(const ss::JsonField& field) {
    if (ss::JsonType::BOOLEAN != field.type()) {
        throw common::WiltonInternalException(TRACEMSG("Invalid '" + field.name() + "' field,"
                " type: [" + ss::stringify_json_type(field.type()) + "]," +
                " value: [" + ss::dump_json_to_string(field.value()) + "]"));
    }
    return field.as_bool();
}

const std::vector<staticlib::serialization::JsonValue>& get_json_array(
        const staticlib::serialization::JsonField& field) {
    if (ss::JsonType::ARRAY != field.type()) {
        throw common::WiltonInternalException(TRACEMSG("Invalid '" + field.name() + "' field,"
                " type: [" + ss::stringify_json_type(field.type()) + "]," +
                " value: [" + ss::dump_json_to_string(field.value()) + "]"));
    }
    return field.as_array();
}

const std::vector<staticlib::serialization::JsonField>& get_json_object(
        const staticlib::serialization::JsonField& field) {
    if (ss::JsonType::OBJECT != field.type()) {
        throw common::WiltonInternalException(TRACEMSG("Invalid '" + field.name() + "' field,"
                " type: [" + ss::stringify_json_type(field.type()) + "]," +
                " value: [" + ss::dump_json_to_string(field.value()) + "]"));
    }
    return field.as_object();    
}

} //namespace
}  


/* 
 * File:   utils.cpp
 * Author: alex
 * 
 * Created on June 14, 2016, 11:45 AM
 */

#include "common/utils.hpp"

#include "staticlib/utils.hpp"

namespace wilton {
namespace common {

namespace { // anonymous

namespace ss = staticlib::serialization;
namespace su = staticlib::utils;

} //namespace

const std::string& get_json_string(const ss::JsonField& field, const std::string& name) {
    if (ss::JsonType::STRING != field.get_type() || field.get_string().empty()) {
        throw common::WiltonInternalException(TRACEMSG("Invalid '" + name + "' field,"
            " type: [" + ss::stringify_json_type(field.get_type()) + "]," +
            " value: [" + ss::dump_json_to_string(field.get_value()) + "]"));
    }
    return field.get_string();
}

uint32_t get_json_uint32(const staticlib::serialization::JsonField& field, const std::string& name) {
    if (ss::JsonType::INTEGER != field.get_type() || !su::is_uint32(field.get_integer())) {
        throw common::WiltonInternalException(TRACEMSG("Invalid '" + name + "' field,"
                " type: [" + ss::stringify_json_type(field.get_type()) + "]," +
                " value: [" + ss::dump_json_to_string(field.get_value()) + "]"));
    }
    return field.get_uint32();
}

uint16_t get_json_uint16(const staticlib::serialization::JsonField& field, const std::string& name) {
    if (ss::JsonType::INTEGER != field.get_type() || !su::is_uint16(field.get_integer())) {
        throw common::WiltonInternalException(TRACEMSG("Invalid '" + name + "' field,"
                " type: [" + ss::stringify_json_type(field.get_type()) + "]," +
                " value: [" + ss::dump_json_to_string(field.get_value()) + "]"));
    }
    return field.get_uint16();
}

bool get_json_bool(const ss::JsonField& field, const std::string& name) {
    if (ss::JsonType::BOOLEAN != field.get_type()) {
        throw common::WiltonInternalException(TRACEMSG("Invalid '" + name + "' field,"
                " type: [" + ss::stringify_json_type(field.get_type()) + "]," +
                " value: [" + ss::dump_json_to_string(field.get_value()) + "]"));
    }
    return field.get_boolean();
}

const std::vector<staticlib::serialization::JsonValue>& get_json_array(
        const staticlib::serialization::JsonField& field, const std::string& name) {
    if (ss::JsonType::ARRAY != field.get_type() || 0 == field.get_array().size()) {
        throw common::WiltonInternalException(TRACEMSG("Invalid '" + name + "' field,"
                " type: [" + ss::stringify_json_type(field.get_type()) + "]," +
                " value: [" + ss::dump_json_to_string(field.get_value()) + "]"));
    }
    return field.get_array();
}

const std::vector<staticlib::serialization::JsonField>& get_json_object(
        const staticlib::serialization::JsonField& field, const std::string& name) {
    if (ss::JsonType::OBJECT != field.get_type() || 0 == field.get_object().size()) {
        throw common::WiltonInternalException(TRACEMSG("Invalid '" + name + "' field,"
                " type: [" + ss::stringify_json_type(field.get_type()) + "]," +
                " value: [" + ss::dump_json_to_string(field.get_value()) + "]"));
    }
    return field.get_object();    
}

} //namespace
}  


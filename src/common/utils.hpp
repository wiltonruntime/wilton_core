/* 
 * File:   utils.hpp
 * Author: alex
 *
 * Created on June 14, 2016, 11:45 AM
 */

#ifndef WILTON_COMMON_UTILS_HPP
#define	WILTON_COMMON_UTILS_HPP

#include <cstdint>
#include <string>
#include <vector>

#include "staticlib/serialization.hpp"

#include "common/WiltonInternalException.hpp"

namespace wilton {
namespace common {

const std::string& get_json_string(const staticlib::serialization::JsonField& field, const std::string& name);

uint32_t get_json_uint32(const staticlib::serialization::JsonField& field, const std::string& name);

uint16_t get_json_uint16(const staticlib::serialization::JsonField& field, const std::string& name);

const std::vector<staticlib::serialization::JsonValue>& get_json_array(
        const staticlib::serialization::JsonField& field, const std::string& name);

const std::vector<staticlib::serialization::JsonField>& get_json_object(
        const staticlib::serialization::JsonField& field, const std::string& name);

} //namespace
}        
        
#endif	/* WILTON_COMMON_UTILS_HPP */


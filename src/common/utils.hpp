/* 
 * File:   utils.hpp
 * Author: alex
 *
 * Created on June 14, 2016, 11:45 AM
 */

#ifndef WILTON_COMMON_UTILS_HPP
#define WILTON_COMMON_UTILS_HPP

#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "staticlib/json.hpp"

#include "common/wilton_internal_exception.hpp"

namespace wilton {
namespace common {

void throw_wilton_error(char* err, const std::string& msg);

void check_json_callback_script(const sl::json::field& field);

void dump_error(const std::string& directory, const std::string& msg);

} //namespace
}        
        
#endif /* WILTON_COMMON_UTILS_HPP */


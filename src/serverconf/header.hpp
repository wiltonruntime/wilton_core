/* 
 * File:   header.hpp
 * Author: alex
 *
 * Created on May 5, 2016, 9:00 PM
 */

#ifndef WILTON_SERVERCONF_HEADER_HPP
#define WILTON_SERVERCONF_HEADER_HPP

#include <string>

#include "staticlib/config.hpp"
#include "staticlib/json.hpp"

#include "wilton/support/exception.hpp"

namespace wilton {
namespace serverconf {

class header {
public:
    std::string name = "";
    std::string value = "";

    header(const header&) = delete;

    header& operator=(const header&) = delete;

    header(header&& other) :
    name(std::move(other.name)),
    value(std::move(other.value)) { }

    header& operator=(header&& other) {
        this->name = std::move(other.name);
        this->value = std::move(other.value);
        return *this;
    }

    header() { }
    
    header(std::string name, std::string value) :
    name(std::move(name)),
    value(std::move(value)) { }

    header(const sl::json::value& json) {
        for (const sl::json::field& fi : json.as_object()) {
            auto& fname = fi.name();
            if ("name" == fname) {
                this->name = fi.as_string_nonempty_or_throw(fname);
            } else if ("value" == fname) {
                this->value = fi.as_string_nonempty_or_throw(fname);
            } else {
                throw support::exception(TRACEMSG("Unknown 'header' field: [" + fname + "]"));
            }
        }
        if (0 == name.length()) throw support::exception(TRACEMSG(
                "Invalid 'header.name' field: []"));
        if (0 == value.length()) throw support::exception(TRACEMSG(
                "Invalid 'header.value' field: []"));
    }

    sl::json::field to_json() const {
        return sl::json::field{name, sl::json::value{value}};
    }
};


} // namepspace
}

#endif /* WILTON_SERVERCONF_HEADER_HPP */


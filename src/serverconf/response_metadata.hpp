/* 
 * File:   response_metadata.hpp
 * Author: alex
 *
 * Created on May 5, 2016, 7:21 PM
 */

#ifndef WILTON_SERVERCONF_RESPONSE_METADATA_HPP
#define WILTON_SERVERCONF_RESPONSE_METADATA_HPP

#include <cstdint>
#include <string>

#include "staticlib/config.hpp"
#include "staticlib/ranges.hpp"
#include "staticlib/json.hpp"

#include "wilton/support/exception.hpp"

#include "serverconf/header.hpp"

namespace wilton {
namespace serverconf {

class response_metadata {
public:
    uint16_t statusCode = 200;
    std::string statusMessage = "OK";
    std::vector<serverconf::header> headers;

    response_metadata(const response_metadata&) = delete;

    response_metadata& operator=(const response_metadata&) = delete;

    response_metadata(response_metadata&& other) :
    statusCode(other.statusCode),
    statusMessage(std::move(other.statusMessage)),
    headers(std::move(other.headers)) { }

    response_metadata& operator=(response_metadata&& other) {
        statusCode = other.statusCode;
        statusMessage = std::move(other.statusMessage);
        headers = std::move(other.headers);
        return *this;
    }
    
    response_metadata(const sl::json::value& json) {
        for (const sl::json::field& fi : json.as_object()) {
            auto& name = fi.name();
            if ("statusCode" == name) {
                this->statusCode = fi.as_uint16_or_throw(name);
            } else if ("statusMessage" == name) {
                this->statusMessage = fi.as_string_nonempty_or_throw(name);
            } else if ("headers" == name) {
                for (const sl::json::field& hf : fi.as_object_or_throw(name)) {
                    std::string val = hf.as_string_nonempty_or_throw(hf.name());
                    this->headers.emplace_back(hf.name(), std::move(val));
                }
            } else {
                throw support::exception(TRACEMSG("Unknown field: [" + name + "]"));
            }
        }
    }

    sl::json::value to_json() const {
        auto ha = sl::ranges::transform(headers, [](const serverconf::header & el) {
            return el.to_json();
        });
        std::vector<sl::json::field> hfields = sl::ranges::emplace_to_vector(std::move(ha));
        return {
            {"statusCode", statusCode},
            {"statusMessage", statusMessage},
            {"headers", std::move(hfields)}
        };
    }
    
};

} // namespace
}

#endif /* WILTON_SERVERCONF_RESPONSE_METADATA_HPP */


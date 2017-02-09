/* 
 * File:   response_metadata.hpp
 * Author: alex
 *
 * Created on May 5, 2016, 7:21 PM
 */

#ifndef WILTON_SERVERCONF_RESPONSE_METADATA_HPP
#define	WILTON_SERVERCONF_RESPONSE_METADATA_HPP

#include <cstdint>
#include <string>

#include "staticlib/config.hpp"
#include "staticlib/ranges.hpp"
#include "staticlib/serialization.hpp"

#include "common/wilton_internal_exception.hpp"
#include "common/utils.hpp"
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
    
    response_metadata(const staticlib::serialization::json_value& json) {
        namespace ss = staticlib::serialization;
        for (const ss::json_field& fi : json.as_object()) {
            auto& name = fi.name();
            if ("statusCode" == name) {
                this->statusCode = common::get_json_uint16(fi);
            } else if ("statusMessage" == name) {
                this->statusMessage = common::get_json_string(fi);
            } else if ("headers" == name) {
                for (const ss::json_field& hf : common::get_json_object(fi)) {
                    std::string val = common::get_json_string(hf);
                    this->headers.emplace_back(hf.name(), std::move(val));
                }
            } else {
                throw common::wilton_internal_exception(TRACEMSG("Unknown field: [" + name + "]"));
            }
        }
    }

    staticlib::serialization::json_value to_json() const {
        namespace sr = staticlib::ranges;
        namespace ss = staticlib::serialization;
        auto ha = sr::transform(sr::refwrap(headers), [](const serverconf::header & el) {
            return el.to_json();
        });
        std::vector<ss::json_field> hfields = sr::emplace_to_vector(std::move(ha));
        return {
            {"statusCode", statusCode},
            {"statusMessage", statusMessage},
            {"headers", std::move(hfields)}
        };
    }
    
};

} // namespace
}

#endif	/* WILTON_SERVERCONF_RESPONSE_METADATA_HPP */


/* 
 * File:   ResponseMetadata.hpp
 * Author: alex
 *
 * Created on May 5, 2016, 7:21 PM
 */

#ifndef WILTON_SERVERCONF_RESPONSEMETADATA_HPP
#define	WILTON_SERVERCONF_RESPONSEMETADATA_HPP

#include <cstdint>
#include <string>

#include "staticlib/config.hpp"
#include "staticlib/ranges.hpp"
#include "staticlib/serialization.hpp"

#include "common/WiltonInternalException.hpp"
#include "common/utils.hpp"
#include "serverconf/Header.hpp"

namespace wilton {
namespace serverconf {

class ResponseMetadata {
public:
    uint16_t statusCode = 200;
    std::string statusMessage = "OK";
    std::vector<serverconf::Header> headers;

    ResponseMetadata(const ResponseMetadata&) = delete;

    ResponseMetadata& operator=(const ResponseMetadata&) = delete;

    ResponseMetadata(ResponseMetadata&& other) :
    statusCode(other.statusCode),
    statusMessage(std::move(other.statusMessage)),
    headers(std::move(other.headers)) { }

    ResponseMetadata& operator=(ResponseMetadata&& other) {
        statusCode = other.statusCode;
        statusMessage = std::move(other.statusMessage);
        headers = std::move(other.headers);
        return *this;
    }
    
    ResponseMetadata(const staticlib::serialization::JsonValue& json) {
        namespace ss = staticlib::serialization;
        for (const ss::JsonField& fi : json.as_object()) {
            auto& name = fi.name();
            if ("statusCode" == name) {
                this->statusCode = common::get_json_uint16(fi, "statusCode");
            } else if ("statusMessage" == name) {
                this->statusMessage = common::get_json_string(fi, "statusMessage");
            } else if ("headers" == name) {
                for (const ss::JsonField& hf : common::get_json_object(fi, "headers")) {
                    std::string val = common::get_json_string(hf, std::string("headers.") + hf.name());
                    this->headers.emplace_back(hf.name(), std::move(val));
                }
            } else {
                throw common::WiltonInternalException(TRACEMSG("Unknown field: [" + name + "]"));
            }
        }
    }

    staticlib::serialization::JsonValue to_json() const {
        namespace sr = staticlib::ranges;
        namespace ss = staticlib::serialization;
        auto ha = sr::transform(sr::refwrap(headers), [](const serverconf::Header & el) {
            return el.to_json();
        });
        std::vector<ss::JsonField> hfields = sr::emplace_to_vector(std::move(ha));
        return {
            {"statusCode", statusCode},
            {"statusMessage", statusMessage},
            {"headers", std::move(hfields)}
        };
    }
    
};

} // namespace
}

#endif	/* WILTON_SERVERCONF_RESPONSEMETADATA_HPP */


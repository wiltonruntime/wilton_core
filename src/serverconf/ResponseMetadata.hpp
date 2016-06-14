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
        for (const ss::JsonField& fi : json.get_object()) {
            auto& name = fi.get_name();
            if ("statusCode" == name) {
                if (ss::JsonType::INTEGER != fi.get_type() ||
                        fi.get_int32() < 0 ||
                        fi.get_uint32() > std::numeric_limits<uint16_t>::max()) {
                    throw common::WiltonInternalException(TRACEMSG(
                            "Invalid 'statusCode' field: [" + ss::dump_json_to_string(fi.get_value()) + "]"));
                }
                this->statusCode = fi.get_uint16();
            } else if ("statusMessage" == name) {
                if (0 == fi.get_string().length()) throw common::WiltonInternalException(TRACEMSG(
                        "Invalid 'statusMessage' field: [" + ss::dump_json_to_string(fi.get_value()) + "]"));
                this->statusMessage = fi.get_string();
            } else if ("headers" == name) {
                if (ss::JsonType::OBJECT != fi.get_type()) throw common::WiltonInternalException(TRACEMSG(
                        "Invalid 'headers' field: [" + ss::dump_json_to_string(fi.get_value()) + "]"));
                for (const auto& hf : fi.get_object()) {
                    if (ss::JsonType::STRING != hf.get_type()) throw common::WiltonInternalException(TRACEMSG(
                            "Invalid 'headers' field: [" + ss::dump_json_to_string(fi.get_value()) + "]"));
                    this->headers.emplace_back(hf.get_name(), hf.get_string());
                }
            } else {
                throw common::WiltonInternalException(TRACEMSG(
                        "Unknown field: [" + name + "]"));
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


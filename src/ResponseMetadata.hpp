/* 
 * File:   ResponseMetadata.hpp
 * Author: alex
 *
 * Created on May 5, 2016, 7:21 PM
 */

#ifndef WILTON_C_RESPONSEMETADATA_HPP
#define	WILTON_C_RESPONSEMETADATA_HPP

#include <cstdint>
#include <string>

#include "staticlib/config.hpp"
#include "staticlib/httpserver/http_message.hpp"
#include "staticlib/serialization.hpp"

#include "WiltonInternalException.hpp"

namespace wilton {
namespace c {

namespace { // anonymous

namespace sh = staticlib::httpserver;
namespace ss = staticlib::serialization;

}

class ResponseMetadata {
public:
    uint16_t statusCode = sh::http_message::RESPONSE_CODE_OK;
    std::string statusMessage = sh::http_message::RESPONSE_MESSAGE_OK;

    /*
    "headers" :{
    "Header-Name" : "header_value",
    ...
    }    
     * */
    
    ResponseMetadata(const JsonValue& json) {
        for (const auto& fi : json.get_object()) {
            auto name = fi.get_name();
            if ("statusCode" == name) {
                if (ss::JsonType::INTEGER != fi.get_type() ||
                        fi.get_int32() < 0 ||
                        fi.get_uint32() > std::numeric_limits<uint16_t>::max()) {
                    throw WiltonInternalException(TRACEMSG(std::string() +
                            "Invalid 'statusCode' field: [" + ss::dump_json_to_string(fi) + "]"));
                }
                this->statusMessage = fi.get_uint16();
            } else if ("statusMessage" == name) {
                if (0 == fi.get_string().length()) throw WiltonInternalException(TRACEMSG(std::string() +
                        "Invalid 'statusMessage' field: [" + ss::dump_json_to_string(fi) + "]"));
                this->statusMessage = fi.get_string();
            } else {
                throw WiltonInternalException(TRACEMSG(std::string() +
                        "Unknown field: [" + ss::dump_json_to_string(fi) + "]"));
            }
        }
    }

    ss::JsonValue to_json() {
        return {
            {"statusCode", statusCode},
            {"statusMessage", statusMessage}
        }
    }
    
};

} // namespace
}

#endif	/* WILTON_C_RESPONSEMETADATA_HPP */


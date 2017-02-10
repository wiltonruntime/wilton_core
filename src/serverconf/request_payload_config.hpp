/* 
 * File:   request_payload_config.hpp
 * Author: alex
 *
 * Created on June 29, 2016, 12:22 PM
 */

#ifndef WILTON_SERVERCONF_REQUEST_PAYLOAD_CONFIG_HPP
#define	WILTON_SERVERCONF_REQUEST_PAYLOAD_CONFIG_HPP

#include <cstdint>
#include <string>

#include "common/wilton_internal_exception.hpp"
#include "common/utils.hpp"
#include "staticlib/serialization.hpp"

namespace wilton {
namespace serverconf {

class request_payload_config {
public:
    std::string tmpDirPath;
    uint16_t tmpFilenameLength = 32;
    uint32_t memoryLimitBytes = 1048576;

    request_payload_config(const request_payload_config&) = delete;

    request_payload_config& operator=(const request_payload_config&) = delete;

    request_payload_config(request_payload_config&& other) :
    tmpDirPath(std::move(other.tmpDirPath)),
    tmpFilenameLength(other.tmpFilenameLength),
    memoryLimitBytes(other.memoryLimitBytes) { }

    request_payload_config& operator=(request_payload_config&& other) {
        this->tmpDirPath = std::move(other.tmpDirPath);
        this->tmpFilenameLength = other.tmpFilenameLength;
        this->memoryLimitBytes = other.memoryLimitBytes;
        return *this;
    }

    request_payload_config() { }
    
    request_payload_config(const std::string& tmpDirPath, uint16_t tmpFilenameLen, uint32_t memoryLimitBytes) :
    tmpDirPath(tmpDirPath.c_str(), tmpDirPath.length()),
    tmpFilenameLength(tmpFilenameLen),
    memoryLimitBytes(memoryLimitBytes) { }

    request_payload_config(const staticlib::serialization::json_value& json) {
        namespace ss = staticlib::serialization;
        for (const ss::json_field& fi : json.as_object()) {
            auto& name = fi.name();
            if ("tmpDirPath" == name) {
                this->tmpDirPath = fi.as_string_nonempty_or_throw(name);
            } else if ("tmpFilenameLength" == name) {
                this->tmpFilenameLength = fi.as_uint16_or_throw(name);
            } else if ("memoryLimitBytes" == name) {
                this->memoryLimitBytes = fi.as_uint32_or_throw(name);
            } else {
                throw common::wilton_internal_exception(TRACEMSG("Unknown 'requestPayload' field: [" + name + "]"));
            }
        }
    }

    staticlib::serialization::json_value to_json() const {
        return {
            { "tmpDirPath", tmpDirPath },
            { "tmpFilenameLen", tmpFilenameLength },
            { "memoryLimitBytes", memoryLimitBytes }
        };
    }

    request_payload_config clone() const {
        return request_payload_config{tmpDirPath, tmpFilenameLength, memoryLimitBytes};
    }
    
};

} // namespace
}

#endif	/* WILTON_SERVERCONF_REQUEST_PAYLOAD_CONFIG_HPP */


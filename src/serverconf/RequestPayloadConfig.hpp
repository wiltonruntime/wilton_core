/* 
 * File:   RequestPayloadConfig.hpp
 * Author: alex
 *
 * Created on June 29, 2016, 12:22 PM
 */

#ifndef WILTON_SERVERCONF_REQUESTPAYLOADCONFIG_HPP
#define	WILTON_SERVERCONF_REQUESTPAYLOADCONFIG_HPP

#include <cstdint>
#include <string>

#include "common/WiltonInternalException.hpp"
#include "common/utils.hpp"
#include "staticlib/serialization.hpp"

namespace wilton {
namespace serverconf {

class RequestPayloadConfig {
public:
    std::string tmpDirPath;
    uint16_t tmpFilenameLength = 32;
    uint32_t memoryLimitBytes = 1048576;

    RequestPayloadConfig(const RequestPayloadConfig&) = delete;

    RequestPayloadConfig& operator=(const RequestPayloadConfig&) = delete;

    RequestPayloadConfig(RequestPayloadConfig&& other) :
    tmpDirPath(std::move(other.tmpDirPath)),
    tmpFilenameLength(other.tmpFilenameLength),
    memoryLimitBytes(other.memoryLimitBytes) { }

    RequestPayloadConfig& operator=(RequestPayloadConfig&& other) {
        this->tmpDirPath = std::move(other.tmpDirPath);
        this->tmpFilenameLength = other.tmpFilenameLength;
        this->memoryLimitBytes = other.memoryLimitBytes;
        return *this;
    }

    RequestPayloadConfig() { }
    
    RequestPayloadConfig(const std::string& tmpDirPath, uint16_t tmpFilenameLen, uint32_t memoryLimitBytes) :
    tmpDirPath(tmpDirPath.c_str(), tmpDirPath.length()),
    tmpFilenameLength(tmpFilenameLen),
    memoryLimitBytes(memoryLimitBytes) { }

    RequestPayloadConfig(const staticlib::serialization::JsonValue& json) {
        namespace ss = staticlib::serialization;
        for (const ss::JsonField& fi : json.get_object()) {
            auto& name = fi.get_name();
            if ("tmpDirPath" == name) {
                this->tmpDirPath = common::get_json_string(fi, "requestPayload.tmpDirPath");
            } else if ("tmpFilenameLength" == name) {
                this->tmpFilenameLength = common::get_json_uint16(fi, "requestPayload.tmpFilenameLength");                
            } else if ("memoryLimitBytes" == name) {
                this->memoryLimitBytes = common::get_json_uint32(fi, "requestPayload.memoryLimitBytes");
            } else {
                throw common::WiltonInternalException(TRACEMSG("Unknown 'requestPayload' field: [" + name + "]"));
            }
        }
    }

    staticlib::serialization::JsonValue to_json() const {
        return {
            { "tmpDirPath", tmpDirPath },
            { "tmpFilenameLen", tmpFilenameLength },
            { "memoryLimitBytes", memoryLimitBytes }
        };
    }

    RequestPayloadConfig clone() const {
        return RequestPayloadConfig{tmpDirPath, tmpFilenameLength, memoryLimitBytes};
    }
    
};

} // namespace
}

#endif	/* WILTON_SERVERCONF_REQUESTPAYLOADCONFIG_HPP */


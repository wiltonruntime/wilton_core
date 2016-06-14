/* 
 * File:   ClientSessionConfig.hpp
 * Author: alex
 *
 * Created on June 13, 2016, 4:24 PM
 */

#ifndef WILTON_CLIENT_CLIENTSESSIONCONFIG_HPP
#define	WILTON_CLIENT_CLIENTSESSIONCONFIG_HPP

#include <cstdint>
#include <string>

#include "staticlib/httpclient.hpp"
#include "staticlib/serialization.hpp"

#include "common/WiltonInternalException.hpp"

namespace wilton {
namespace client {

class ClientSessionConfig {
public:
    staticlib::httpclient::HttpSessionOptions options;

    ClientSessionConfig(const ClientSessionConfig&) = delete;

    ClientSessionConfig& operator=(const ClientSessionConfig&) = delete;

    ClientSessionConfig(ClientSessionConfig&& other) :
    options(std::move(other.options)) { }

    ClientSessionConfig& operator=(ClientSessionConfig&& other) {
        this->options = std::move(other.options);
        return *this;
    }

    ClientSessionConfig() { }

    ClientSessionConfig(const staticlib::serialization::JsonValue& json) {
        namespace ss = staticlib::serialization;
        for (const ss::JsonField& fi : json.get_object()) {
            auto& name = fi.get_name();
            if ("maxHostConnections" == name) {
                if (ss::JsonType::INTEGER != fi.get_type() ||
                        fi.get_int32() < 0 ||
                        fi.get_integer() > std::numeric_limits<uint32_t>::max()) {
                    throw common::WiltonInternalException(TRACEMSG(
                            "Invalid 'maxHostConnections' field: [" + ss::dump_json_to_string(fi.get_value()) + "]"));
                }
                this->options.max_host_connections = fi.get_uint32();
            } else if ("maxTotalConnections" == name) {
                if (ss::JsonType::INTEGER != fi.get_type() ||
                        fi.get_int32() < 0 ||
                        fi.get_integer() > std::numeric_limits<uint32_t>::max()) {
                    throw common::WiltonInternalException(TRACEMSG(
                            "Invalid 'maxTotalConnections' field: [" + ss::dump_json_to_string(fi.get_value()) + "]"));
                }
                this->options.max_total_connections = fi.get_uint32();
            } else if ("maxconnects" == name) {
                if (ss::JsonType::INTEGER != fi.get_type() ||
                        fi.get_int32() < 0 ||
                        fi.get_integer() > std::numeric_limits<uint32_t>::max()) {
                    throw common::WiltonInternalException(TRACEMSG(
                            "Invalid 'maxconnects' field: [" + ss::dump_json_to_string(fi.get_value()) + "]"));
                }
                this->options.maxconnects = fi.get_uint32();
            } else {
                throw common::WiltonInternalException(TRACEMSG(
                        "Unknown 'logging.appenders' field: [" + name + "]"));
            }
        }
    }

    staticlib::serialization::JsonValue to_json() const {
        return {
            { "maxHostConnections", options.max_host_connections },
            { "maxTotalConnections", options.max_total_connections },
            { "maxconnects", options.maxconnects }
        };
    }
    
    
    
    
};

} // namespace
} 

#endif	/* WILTON_CLIENT_CLIENTSESSIONCONFIG_HPP */


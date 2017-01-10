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
#include "common/utils.hpp"

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
        // msvs complains to different vectors in foreach here
        auto& vec = json.as_object();
        for (size_t i = 0; i < vec.size(); i++) {
            const ss::JsonField& fi = vec[i];
            auto& name = fi.name();
            if ("maxHostConnections" == name) {
                this->options.max_host_connections = common::get_json_uint32(fi);
            } else if ("maxTotalConnections" == name) {
                this->options.max_total_connections = common::get_json_uint32(fi);
            } else if ("maxconnects" == name) {
                this->options.maxconnects = common::get_json_uint32(fi);
            } else {
                throw common::WiltonInternalException(TRACEMSG("Unknown 'ClientSession' field: [" + name + "]"));
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


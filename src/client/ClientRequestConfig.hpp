/* 
 * File:   ClientRequestConfig.hpp
 * Author: alex
 *
 * Created on June 13, 2016, 4:24 PM
 */

#ifndef WILTON_CLIENT_CLIENTREQUESTCONFIG_HPP
#define	WILTON_CLIENT_CLIENTREQUESTCONFIG_HPP

#include <cstdint>
#include <string>

#include "staticlib/httpclient.hpp"
#include "staticlib/serialization.hpp"

#include "common/WiltonInternalException.hpp"

namespace wilton {
namespace client {

class ClientRequestConfig {
public:
    staticlib::httpclient::HttpRequestOptions options;

    ClientRequestConfig(const ClientRequestConfig&) = delete;

    ClientRequestConfig& operator=(const ClientRequestConfig&) = delete;

    ClientRequestConfig(ClientRequestConfig&& other) :
    options(std::move(other.options)) { }

    ClientRequestConfig& operator=(ClientRequestConfig&& other) {
        this->options = std::move(other.options);
        return *this;
    }

    ClientRequestConfig() { }

    ClientRequestConfig(const staticlib::serialization::JsonValue& json) {
        namespace ss = staticlib::serialization;
        for (const ss::JsonField& fi : json.get_object()) {
            auto& name = fi.get_name();
            if ("maxHostConnections" == name) {
            } else if ("maxconnects" == name) {
            } else {
                throw common::WiltonInternalException(TRACEMSG(
                        "Unknown 'logging.appenders' field: [" + name + "]"));
            }
        }
    }

    staticlib::serialization::JsonValue to_json() const {
        return
        {
            {
                "maxHostConnections", options.max_host_connections
            },
            {
                "maxTotalConnections", options.max_total_connections
            },
            {
                "maxconnects", options.maxconnects
            }
        };
    }

};

} // namespace
} 

#endif	/* WILTON_CLIENT_CLIENTREQUESTCONFIG_HPP */


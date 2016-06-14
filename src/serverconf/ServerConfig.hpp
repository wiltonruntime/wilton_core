/* 
 * File:   ServerConfigConfig.hpp
 * Author: alex
 *
 * Created on May 5, 2016, 7:20 PM
 */

#ifndef WILTON_SERVERCONF_SERVERCONFIG_HPP
#define	WILTON_SERVERCONF_SERVERCONFIG_HPP

#include <cstdint>
#include <string>
#include <vector>

#include "staticlib/ranges.hpp"
#include "staticlib/serialization.hpp"

#include "common/WiltonInternalException.hpp"
#include "serverconf/DocumentRoot.hpp"
#include "serverconf/Appender.hpp"
#include "serverconf/Logging.hpp"
#include "serverconf/SslConfig.hpp"

namespace wilton {
namespace serverconf {

class ServerConfig {
public:    
    uint32_t numberOfThreads = 2;
    uint16_t tcpPort = 8080;
    std::string ipAddress = "0.0.0.0";
    SslConfig ssl;
    std::vector<DocumentRoot> documentRoots;
    Logging logging;

    ServerConfig(const ServerConfig&) = delete;

    ServerConfig& operator=(const ServerConfig&) = delete;

    ServerConfig(ServerConfig&& other) :
    numberOfThreads(other.numberOfThreads),
    tcpPort(other.tcpPort),
    ipAddress(std::move(other.ipAddress)),
    ssl(std::move(other.ssl)),
    documentRoots(std::move(other.documentRoots)),
    logging(std::move(other.logging)) { }

    ServerConfig& operator=(ServerConfig&& other) {
        this->numberOfThreads = other.numberOfThreads;
        this->tcpPort = other.tcpPort;
        this->ipAddress = std::move(other.ipAddress);
        this->ssl = std::move(other.ssl);
        this->documentRoots = std::move(other.documentRoots);
        this->logging = std::move(other.logging);
        return *this;
    }

    ServerConfig(const staticlib::serialization::JsonValue& json) {
        namespace ss = staticlib::serialization;
        for (const ss::JsonField& fi : json.get_object()) {
            auto& name = fi.get_name();
            if ("numberOfThreads" == name) {
                if (ss::JsonType::INTEGER != fi.get_type() ||
                        fi.get_int32() < 0 ||
                        fi.get_uint32() > std::numeric_limits<uint16_t>::max()) {
                    throw common::WiltonInternalException(TRACEMSG(
                            "Invalid 'numberOfThreads' field: [" + ss::dump_json_to_string(fi.get_value()) + "]"));
                }
                this->numberOfThreads = fi.get_uint16();
            } else if ("tcpPort" == name) {
                if (ss::JsonType::INTEGER != fi.get_type() ||
                        fi.get_int32() < 0 ||
                        fi.get_integer() > std::numeric_limits<uint32_t>::max()) {
                    throw common::WiltonInternalException(TRACEMSG(
                            "Invalid 'tcpPort' field: [" + ss::dump_json_to_string(fi.get_value()) + "]"));
                }
                this->tcpPort = fi.get_uint32();
            } else if ("ipAddress" == name) {
                this->ipAddress = fi.get_string();
            } else if ("ssl" == name) {
                this->ssl = SslConfig(fi.get_value());
            } else if ("documentRoots" == name) {
                if (ss::JsonType::ARRAY != fi.get_type() || 0 == fi.get_array().size()) throw common::WiltonInternalException(TRACEMSG(
                        "Invalid 'documentRoots' field: [" + ss::dump_json_to_string(fi.get_value()) + "]"));
                for (const ss::JsonValue& lo : fi.get_array()) {
                    auto jd = serverconf::DocumentRoot(lo);
                    this->documentRoots.emplace_back(std::move(jd));
                }
            } else if ("logging" == name) {
                this->logging = Logging(fi.get_value());
            } else {
                throw common::WiltonInternalException(TRACEMSG(
                        "Unknown field: [" + name + "]"));
            }
        }
        if (0 == logging.appenders.size()) {
            logging.appenders.emplace_back(serverconf::Appender());
        }
    }
    
    staticlib::serialization::JsonValue to_json() const {
        namespace sr = staticlib::ranges;
        auto drs = sr::transform(sr::refwrap(documentRoots), [](const serverconf::DocumentRoot& el) {
            return el.to_json();
        });
        return {
            {"numberOfThreads", numberOfThreads},
            {"tcpPort", tcpPort},
            {"ipAddress", ipAddress},
            {"ssl", ssl.to_json()},
            {"documentRoots", drs},
            {"logging", logging.to_json()}
        };
    }
};

} // namespace
}

#endif	/* WILTON_SERVERCONF_SERVERCONFIG_HPP */


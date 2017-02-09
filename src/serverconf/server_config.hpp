/* 
 * File:   server_configConfig.hpp
 * Author: alex
 *
 * Created on May 5, 2016, 7:20 PM
 */

#ifndef WILTON_SERVERCONF_SERVER_CONFIG_HPP
#define	WILTON_SERVERCONF_SERVER_CONFIG_HPP

#include <cstdint>
#include <string>
#include <vector>

#include "staticlib/ranges.hpp"
#include "staticlib/serialization.hpp"

#include "common/wilton_internal_exception.hpp"
#include "common/utils.hpp"
#include "serverconf/document_root.hpp"
#include "serverconf/mustache_config.hpp"
#include "serverconf/request_payload_config.hpp"
#include "serverconf/ssl_config.hpp"

namespace wilton {
namespace serverconf {

class server_config {
public:    
    uint32_t numberOfThreads = 2;
    uint16_t tcpPort = 8080;
    std::string ipAddress = "0.0.0.0";
    ssl_config ssl;
    std::vector<document_root> documentRoots;
    request_payload_config requestPayload;
    mustache_config mustache;

    server_config(const server_config&) = delete;

    server_config& operator=(const server_config&) = delete;

    server_config(server_config&& other) :
    numberOfThreads(other.numberOfThreads),
    tcpPort(other.tcpPort),
    ipAddress(std::move(other.ipAddress)),
    ssl(std::move(other.ssl)),
    documentRoots(std::move(other.documentRoots)),
    requestPayload(std::move(other.requestPayload)),
    mustache(std::move(other.mustache)) { }

    server_config& operator=(server_config&& other) {
        this->numberOfThreads = other.numberOfThreads;
        this->tcpPort = other.tcpPort;
        this->ipAddress = std::move(other.ipAddress);
        this->ssl = std::move(other.ssl);
        this->documentRoots = std::move(other.documentRoots);
        this->requestPayload = std::move(other.requestPayload);
        this->mustache = std::move(other.mustache);
        return *this;
    }

    server_config(const staticlib::serialization::json_value& json) {
        namespace ss = staticlib::serialization;
        for (const ss::json_field& fi : json.as_object()) {
            auto& name = fi.name();
            if ("numberOfThreads" == name) {
                this->numberOfThreads = common::get_json_uint16(fi);
            } else if ("tcpPort" == name) {
                this->tcpPort = common::get_json_uint16(fi);
            } else if ("ipAddress" == name) {
                this->ipAddress = common::get_json_string(fi);
            } else if ("ssl" == name) {
                this->ssl = ssl_config(fi.value());
            } else if ("documentRoots" == name) {
                for (const ss::json_value& lo : common::get_json_array(fi)) {
                    auto jd = serverconf::document_root(lo);
                    this->documentRoots.emplace_back(std::move(jd));
                }
            } else if ("requestPayload" == name) {
                this->requestPayload = serverconf::request_payload_config(fi.value());
            } else if ("mustache" == name) {
                this->mustache = mustache_config(fi.value());
            } else {
                throw common::wilton_internal_exception(TRACEMSG("Unknown field: [" + name + "]"));
            }
        }
    }
    
    staticlib::serialization::json_value to_json() const {
        namespace sr = staticlib::ranges;
        return {
            {"numberOfThreads", numberOfThreads},
            {"tcpPort", tcpPort},
            {"ipAddress", ipAddress},
            {"ssl", ssl.to_json()},
            {"documentRoots", [this]() {
                auto drs = sr::transform(sr::refwrap(documentRoots), [](const serverconf::document_root& el) {
                    return el.to_json();
                });
                return drs.to_vector();
            }()},
            {"requestPayload", requestPayload.to_json()},
            {"mustache", mustache.to_json()}
        };
    }
};

} // namespace
}

#endif	/* WILTON_SERVERCONF_SERVER_CONFIG_HPP */


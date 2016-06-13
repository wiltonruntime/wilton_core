/* 
 * File:   RequestMetadata.hpp
 * Author: alex
 *
 * Created on May 5, 2016, 7:20 PM
 */

#ifndef WILTON_SERVERCONF_REQUESTMETADATA_HPP
#define	WILTON_SERVERCONF_REQUESTMETADATA_HPP

#include <cstdint>
#include <string>
#include <vector>

#include "staticlib/config.hpp"
#include "staticlib/ranges.hpp"
#include "staticlib/serialization.hpp"

#include "common/WiltonInternalException.hpp"
#include "serverconf/Header.hpp"

namespace wilton {
namespace serverconf {

class RequestMetadata {
    std::string httpVersion;
    std::string protocol;
    std::string method;
    std::string pathname;
    std::string query;
    std::vector<serverconf::Header> headers;

public:
    RequestMetadata(const RequestMetadata&) = delete;
    
    RequestMetadata& operator=(const RequestMetadata&) = delete;
    
    RequestMetadata(RequestMetadata&& other) :
    httpVersion(std::move(other.httpVersion)),
    protocol(std::move(other.protocol)),
    method(std::move(other.method)),
    pathname(std::move(other.pathname)),
    query(std::move(other.query)),
    headers(std::move(other.headers)) { }
    
    RequestMetadata& operator=(RequestMetadata&& other) {
        httpVersion = std::move(other.httpVersion);
        protocol = std::move(other.protocol);
        method = std::move(other.method);
        pathname = std::move(other.pathname);
        query = std::move(other.query);
        headers = std::move(other.headers);
        return *this;
    }
    
    RequestMetadata(const std::string& httpVersion, const std::string& protocol, 
            const std::string& method, const std::string& pathname,
            const std::string& query, std::vector<serverconf::Header> headers) :
    httpVersion(httpVersion.data(), httpVersion.length()),
    protocol(protocol.data(), protocol.length()),
    method(method.data(), method.length()),
    pathname(pathname.data(), pathname.length()),
    query(query.data(), query.length()),
    headers(std::move(headers)) { }
        
    staticlib::serialization::JsonValue to_json() const {
        namespace ss = staticlib::serialization;
        namespace sr = staticlib::ranges;
        auto ha = sr::transform(sr::refwrap(headers), [](const serverconf::Header& el) {
            return el.to_json();
        });
        std::vector<ss::JsonField> hfields = sr::emplace_to_vector(std::move(ha));
        return {
            {"httpVersion", httpVersion},
            {"protocol", protocol},
            {"method", method},
            {"pathname", pathname},
            {"query", query},
            {"url", reconstructUrl()},
            {"headers", std::move(hfields)}
        };
    }

private:
    std::string reconstructUrl() const {
        if (0 == query.length()) {
            return pathname;
        } else {
            return pathname + "?" + query;
        }
    }
};


} // namespace
}

#endif	/* WILTON_SERVERCONF_REQUESTMETADATA_HPP */


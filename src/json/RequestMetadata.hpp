/* 
 * File:   RequestMetadata.hpp
 * Author: alex
 *
 * Created on May 5, 2016, 7:20 PM
 */

#ifndef WILTON_C_JSON_REQUESTMETADATA_HPP
#define	WILTON_C_JSON_REQUESTMETADATA_HPP

#include <cstdint>
#include <string>
#include <vector>

#include "staticlib/config.hpp"
#include "staticlib/ranges.hpp"
#include "staticlib/serialization.hpp"

#include "WiltonInternalException.hpp"

#include "json/Header.hpp"

namespace wilton {
namespace c {
namespace json {

namespace { // anonymous

namespace sr = staticlib::ranges;
namespace ss = staticlib::serialization;

}

class RequestMetadata {
    std::string httpVersion;
    std::string method;
    std::string pathname;
    std::string query;
    std::vector<json::Header> headers;

    /*
     * "queryParams": {}
 "remoteIp"
"headers" :
{
    "Header-Name" : "header_value",
    ...
}
     */
    
public:
    RequestMetadata(const RequestMetadata&) = delete;
    
    RequestMetadata& operator=(const RequestMetadata&) = delete;
    
    RequestMetadata(RequestMetadata&& other) :
    httpVersion(std::move(other.httpVersion)),
    method(std::move(other.method)),
    pathname(std::move(other.pathname)),
    query(std::move(other.query)),
    headers(std::move(other.headers)) { }
    
    RequestMetadata& operator=(RequestMetadata&& other) {
        httpVersion = std::move(other.httpVersion);
        method = std::move(other.method);
        pathname = std::move(other.pathname);
        query = std::move(other.query);
        headers = std::move(other.headers);
        return *this;
    }
    
    RequestMetadata(std::string httpVersion, std::string method, std::string pathname,
            std::string query, std::vector<json::Header> headers) :
    httpVersion(std::move(httpVersion)),
    method(std::move(method)),
    pathname(std::move(pathname)),
    query(std::move(query)),
    headers(std::move(headers)) { }
        
    ss::JsonValue to_json() const {
        auto ha = sr::transform(sr::refwrap(headers), [](const json::Header& el) {
            return el.to_json();
        });
        std::vector<ss::JsonField> hfields = sr::emplace_to_vector(std::move(ha));
        return {
            {"httpVersion", httpVersion},
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
}

#endif	/* WILTON_C_JSON_REQUESTMETADATA_HPP */


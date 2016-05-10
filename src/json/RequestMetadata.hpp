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

#include "staticlib/config.hpp"
#include "staticlib/serialization.hpp"

#include "WiltonInternalException.hpp"

namespace wilton {
namespace c {
namespace json {

namespace { // anonymous

namespace ss = staticlib::serialization;

}

class RequestMetadata {
    std::string httpVersion;
    std::string method;
    std::string pathname;
    std::string query;
    

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
    query(std::move(other.query)) { }
    
    RequestMetadata& operator=(RequestMetadata&& other) {
        httpVersion = std::move(other.httpVersion);
        method = std::move(other.method);
        pathname = std::move(other.pathname);
        query = std::move(other.query);
        return *this;
    }
    
    RequestMetadata(std::string httpVersion, std::string method, std::string pathname,
            std::string query) :
    httpVersion(std::move(httpVersion)),
    method(std::move(method)),
    pathname(std::move(pathname)),
    query(std::move(query)) { }
        
    ss::JsonValue to_json() {
        return {
            {"httpVersion", httpVersion},
            {"method", method},
            {"pathname", pathname},
            {"query", query},
            {"url", reconstructUrl()},
        };
    }

private:
    std::string reconstructUrl() {
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


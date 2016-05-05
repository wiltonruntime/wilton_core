/* 
 * File:   RequestMetadata.hpp
 * Author: alex
 *
 * Created on May 5, 2016, 7:20 PM
 */

#ifndef WILTON_C_REQUESTMETADATA_HPP
#define	WILTON_C_REQUESTMETADATA_HPP

#include <cstdint>
#include <string>

#include "staticlib/config.hpp"
#include "staticlib/serialization.hpp"

#include "WiltonInternalException.hpp"

namespace wilton {
namespace c {

namespace { // anonymous

namespace ss = staticlib::serialization;

}

class RequestMetadata {
    std::string httpVersion;
    std::string method;
    std::string resource;
    std::string queryString;

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
    RequestMetadata(std::string httpVersion, std::string method, std::string resource,
            std::string queryString) :
    httpVersion(std::move(httpVersion)),
    method(std::move(method)),
    resource(std::move(resource)),
    queryString(std::move(queryString)) { }
        
    ss::JsonValue to_json() {
        return {
            {"httpVersion", httpVersion},
            {"method", method},
            {"resource", resource},
            {"queryString", queryString},
        }
    }

};


} // namespace
}

#endif	/* WILTON_C_REQUESTMETADATA_HPP */


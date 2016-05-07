/* 
 * File:   DocumentRoot.hpp
 * Author: alex
 *
 * Created on May 5, 2016, 7:27 PM
 */

#ifndef WILTON_C_DOCUMENTROOT_HPP
#define	WILTON_C_DOCUMENTROOT_HPP

#include <cstdint>
#include <string>

#include "asio.hpp"

#include "staticlib/config.hpp"
#include "staticlib/serialization.hpp"

#include "WiltonInternalException.hpp"

namespace wilton {
namespace c {

namespace { // anonymous

namespace ss = staticlib::serialization;

}

class DocumentRoot {
public:
    std::string resource = "";
    std::string directory = "";
    uint32_t cacheMaxAgeSeconds = 604800;
    
    DocumentRoot(const DocumentRoot&) = delete;
    
    DocumentRoot& operator=(const DocumentRoot&) = delete;

    DocumentRoot(DocumentRoot&& other) :
    resource(std::move(other.resource)),
    directory(std::move(other.directory)),
    cacheMaxAgeSeconds(std::move(other.cacheMaxAgeSeconds)) { }

    DocumentRoot& operator=(DocumentRoot&& other) {
        this->resource = std::move(other.resource);
        this->directory = std::move(other.directory);
        this->cacheMaxAgeSeconds = other.cacheMaxAgeSeconds;
        return *this;
    }
    
    DocumentRoot() { }
    
    DocumentRoot(const ss::JsonValue& json) {
        for (const ss::JsonField& fi : json.get_object()) {
            auto& name = fi.get_name();
            if ("resource" == name) {
                if (0 == fi.get_string().length()) throw WiltonInternalException(TRACEMSG(std::string() +
                        "Invalid 'documentRoot.resource' field: [" + ss::dump_json_to_string(fi.get_value()) + "]"));
                this->resource = fi.get_string();
            } else if ("directory" == name) {
                if (0 == fi.get_string().length()) throw WiltonInternalException(TRACEMSG(std::string() +
                        "Invalid 'documentRoot.directory' field: [" + ss::dump_json_to_string(fi.get_value()) + "]"));
                this->resource = fi.get_string();
            } else if ("cacheMaxAgeSeconds" == name) {
                if (ss::JsonType::INTEGER != fi.get_type() ||
                        fi.get_int32() < 0 ||
                        fi.get_integer() > std::numeric_limits<uint32_t>::max()) {
                    throw WiltonInternalException(TRACEMSG(std::string() +
                            "Invalid 'documentRoot.cacheMaxAgeSeconds' field: [" + ss::dump_json_to_string(fi.get_value()) + "]"));
                }
                this->cacheMaxAgeSeconds = fi.get_uint32();
            } else {
                throw WiltonInternalException(TRACEMSG(std::string() +
                        "Unknown 'documentRoot' field: [" + ss::dump_json_to_string(fi.get_value()) + "]"));
            }
        }
        if (0 == resource.length()) throw WiltonInternalException(TRACEMSG(std::string() +
                    "Invalid 'documentRoot.resource' field: []"));
        if (0 == directory.length()) throw WiltonInternalException(TRACEMSG(std::string() +
                    "Invalid 'documentRoot.directory' field: []"));
    }
       
    ss::JsonValue to_json() {
        return {
            {"resource", resource},
            {"directory", directory},
            {"cacheMaxAgeSeconds", cacheMaxAgeSeconds}
        };
    }
    
    bool is_empty() {
        return 0 == resource.length();
    }
};

} // namespace
}

#endif	/* WILTON_C_DOCUMENTROOT_HPP */


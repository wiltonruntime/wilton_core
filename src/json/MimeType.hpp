/* 
 * File:   MimeType.hpp
 * Author: alex
 *
 * Created on May 11, 2016, 1:23 PM
 */

#ifndef WILTON_C_MIMETYPE_HPP
#define	WILTON_C_MIMETYPE_HPP

#include <string>
#include <vector>

#include "staticlib/serialization.hpp"

namespace wilton {
namespace c {
namespace json {

namespace { // anonymous

namespace sr = staticlib::ranges;
namespace ss = staticlib::serialization;

} // namepspace

class MimeType {
public:
    std::string extension;
    std::string mime;

    MimeType(const MimeType&) = delete;

    MimeType& operator=(const MimeType&) = delete;

    MimeType(MimeType&& other) :
    extension(std::move(other.extension)),
    mime(std::move(other.mime)) { }

    MimeType& operator=(MimeType&& other) {
        this->extension = std::move(other.extension);
        this->mime = std::move(other.mime);
        return *this;
    }

    MimeType() { }
    
    MimeType(std::string extension, std::string mime) :
    extension(std::move(extension)),
    mime(std::move(mime)) { }

    MimeType(const ss::JsonValue& json) {
        for (const ss::JsonField& fi : json.get_object()) {
            auto& name = fi.get_name();
            if ("extension" == name) {
                if (0 == fi.get_string().length()) throw WiltonInternalException(TRACEMSG(std::string() +
                        "Invalid 'mimeType.resource' field: [" + ss::dump_json_to_string(fi.get_value()) + "]"));
                this->extension = fi.get_string();
            } else if ("mime" == name) {
                if (0 == fi.get_string().length()) throw WiltonInternalException(TRACEMSG(std::string() +
                        "Invalid 'mimeType.mime' field: [" + ss::dump_json_to_string(fi.get_value()) + "]"));
                this->mime = fi.get_string();
            } else {
                throw WiltonInternalException(TRACEMSG(std::string() +
                        "Unknown 'mimeType' field: [" + ss::dump_json_to_string(fi.get_value()) + "]"));
            }
        }
        if (0 == extension.length()) throw WiltonInternalException(TRACEMSG(std::string() +
                "Invalid 'mimeType.extension' field: []"));
        if (0 == mime.length()) throw WiltonInternalException(TRACEMSG(std::string() +
                "Invalid 'mimeType.mime' and 'documentRoot.zipPath' field: []"));
    }
    
    ss::JsonValue to_json() const {    
        return {
            {"extension", extension},
            {"mime", mime}
        };
    }
    
    MimeType clone() const {
        return MimeType{extension, mime};
    }
    
};

} // namespace
}
}

#endif	/* WILTON_C_MIMETYPE_HPP */


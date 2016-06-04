/* 
 * File:   MimeType.hpp
 * Author: alex
 *
 * Created on May 11, 2016, 1:23 PM
 */

#ifndef WILTON_MIMETYPE_HPP
#define	WILTON_MIMETYPE_HPP

#include <string>
#include <vector>

#include "common/WiltonInternalException.hpp"
#include "staticlib/serialization.hpp"

namespace wilton {
namespace json {

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
    
    MimeType(const std::string& extension, const std::string& mime) :
    extension(extension.data(), extension.length()),
    mime(mime.data(), mime.length()) { }

    MimeType(const staticlib::serialization::JsonValue& json) {
        namespace ss = staticlib::serialization;
        for (const ss::JsonField& fi : json.get_object()) {
            auto& name = fi.get_name();
            if ("extension" == name) {
                if (0 == fi.get_string().length()) throw common::WiltonInternalException(TRACEMSG(std::string() +
                        "Invalid 'mimeType.resource' field: [" + ss::dump_json_to_string(fi.get_value()) + "]"));
                this->extension = fi.get_string();
            } else if ("mime" == name) {
                if (0 == fi.get_string().length()) throw common::WiltonInternalException(TRACEMSG(std::string() +
                        "Invalid 'mimeType.mime' field: [" + ss::dump_json_to_string(fi.get_value()) + "]"));
                this->mime = fi.get_string();
            } else {
                throw common::WiltonInternalException(TRACEMSG(std::string() +
                        "Unknown 'mimeType' field: [" + name + "]"));
            }
        }
        if (0 == extension.length()) throw common::WiltonInternalException(TRACEMSG(std::string() +
                "Invalid 'mimeType.extension' field: []"));
        if (0 == mime.length()) throw common::WiltonInternalException(TRACEMSG(std::string() +
                "Invalid 'mimeType.mime' and 'documentRoot.zipPath' field: []"));
    }
    
    staticlib::serialization::JsonValue to_json() const {    
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

#endif	/* WILTON_MIMETYPE_HPP */


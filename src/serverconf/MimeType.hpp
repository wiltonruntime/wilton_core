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
#include "common/utils.hpp"
#include "staticlib/serialization.hpp"

namespace wilton {
namespace serverconf {

class MimeType {
public:
    std::string extension = "";
    std::string mime = "";

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

    MimeType(const staticlib::serialization::json_value& json) {
        namespace ss = staticlib::serialization;
        for (const ss::json_field& fi : json.as_object()) {
            auto& name = fi.name();
            if ("extension" == name) {
                this->extension = common::get_json_string(fi);
            } else if ("mime" == name) {
                this->mime = common::get_json_string(fi);
            } else {
                throw common::WiltonInternalException(TRACEMSG("Unknown 'mimeType' field: [" + name + "]"));
            }
        }
        if (0 == extension.length()) throw common::WiltonInternalException(TRACEMSG(
                "Invalid 'mimeType.extension' field: []"));
        if (0 == mime.length()) throw common::WiltonInternalException(TRACEMSG(
                "Invalid 'mimeType.mime' field: []"));
    }
    
    staticlib::serialization::json_value to_json() const {    
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


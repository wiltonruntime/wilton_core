/* 
 * File:   mime_type.hpp
 * Author: alex
 *
 * Created on May 11, 2016, 1:23 PM
 */

#ifndef WILTON_MIME_TYPE_HPP
#define	WILTON_MIME_TYPE_HPP

#include <string>
#include <vector>

#include "common/wilton_internal_exception.hpp"
#include "common/utils.hpp"
#include "staticlib/serialization.hpp"

namespace wilton {
namespace serverconf {

class mime_type {
public:
    std::string extension = "";
    std::string mime = "";

    mime_type(const mime_type&) = delete;

    mime_type& operator=(const mime_type&) = delete;

    mime_type(mime_type&& other) :
    extension(std::move(other.extension)),
    mime(std::move(other.mime)) { }

    mime_type& operator=(mime_type&& other) {
        this->extension = std::move(other.extension);
        this->mime = std::move(other.mime);
        return *this;
    }

    mime_type() { }
    
    mime_type(const std::string& extension, const std::string& mime) :
    extension(extension.data(), extension.length()),
    mime(mime.data(), mime.length()) { }

    mime_type(const staticlib::serialization::json_value& json) {
        namespace ss = staticlib::serialization;
        for (const ss::json_field& fi : json.as_object()) {
            auto& name = fi.name();
            if ("extension" == name) {
                this->extension = common::get_json_string(fi);
            } else if ("mime" == name) {
                this->mime = common::get_json_string(fi);
            } else {
                throw common::wilton_internal_exception(TRACEMSG("Unknown 'mimeType' field: [" + name + "]"));
            }
        }
        if (0 == extension.length()) throw common::wilton_internal_exception(TRACEMSG(
                "Invalid 'mimeType.extension' field: []"));
        if (0 == mime.length()) throw common::wilton_internal_exception(TRACEMSG(
                "Invalid 'mimeType.mime' field: []"));
    }
    
    staticlib::serialization::json_value to_json() const {    
        return {
            {"extension", extension},
            {"mime", mime}
        };
    }
    
    mime_type clone() const {
        return mime_type{extension, mime};
    }
    
};

} // namespace
}

#endif	/* WILTON_MIME_TYPE_HPP */


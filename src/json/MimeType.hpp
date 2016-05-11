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
    
    ss::JsonValue to_json() {    
        return { };
    }
    
    MimeType clone() const {
        return MimeType{};
    }
    
};

} // namespace
}
}

#endif	/* WILTON_C_MIMETYPE_HPP */


/* 
 * File:   MustacheConfig.hpp
 * Author: alex
 *
 * Created on September 7, 2016, 11:22 AM
 */

#ifndef WILTON_SERVERCONF_MUSTACHECONFIG_HPP
#define	WILTON_SERVERCONF_MUSTACHECONFIG_HPP

#include <string>

#include "staticlib/serialization.hpp"

#include "common/WiltonInternalException.hpp"
#include "common/utils.hpp"

namespace wilton {
namespace serverconf {

class MustacheConfig {
public:
    std::vector<std::string> partialsDirs;

    MustacheConfig(const MustacheConfig&) = delete;

    MustacheConfig& operator=(const MustacheConfig&) = delete;

    MustacheConfig(MustacheConfig&& other) :
    partialsDirs(std::move(other.partialsDirs)) { }

    MustacheConfig& operator=(MustacheConfig&& other) {
        this->partialsDirs = std::move(other.partialsDirs);
        return *this;
    }

    MustacheConfig() { }

    MustacheConfig(const staticlib::serialization::JsonValue& json) {
        namespace ss = staticlib::serialization;
        for (const ss::JsonField& fi : json.as_object()) {
            auto& name = fi.name();
            if ("partialsDirs" == name) {
                for (const ss::JsonValue& va : common::get_json_array(fi, "mustache.partialsDirs")) {
                    if (ss::JsonType::STRING != va.type() || va.as_string().empty()) {
                        throw common::WiltonInternalException(TRACEMSG(
                                "Invalid 'mustache.partialsDirs.el' value,"
                                " type: [" + ss::stringify_json_type(va.type()) + "]," +
                                " value: [" + ss::dump_json_to_string(va) + "]"));
                    }
                    partialsDirs.emplace_back(va.as_string());
                }
            } else {
                throw common::WiltonInternalException(TRACEMSG("Unknown 'mustache' field: [" + name + "]"));
            }
        }
    }

    staticlib::serialization::JsonValue to_json() const {
        return {
            { "partialsDirs", partialsDirs }
        };
    }
};

} // namespace
}

#endif	/* WILTON_SERVERCONF_MUSTACHECONFIG_HPP */


/* 
 * File:   mustache_config.hpp
 * Author: alex
 *
 * Created on September 7, 2016, 11:22 AM
 */

#ifndef WILTON_SERVERCONF_MUSTACHE_CONFIG_HPP
#define	WILTON_SERVERCONF_MUSTACHE_CONFIG_HPP

#include <string>

#include "staticlib/serialization.hpp"

#include "common/wilton_internal_exception.hpp"
#include "common/utils.hpp"

namespace wilton {
namespace serverconf {

class mustache_config {
public:
    std::vector<std::string> partialsDirs;

    mustache_config(const mustache_config&) = delete;

    mustache_config& operator=(const mustache_config&) = delete;

    mustache_config(mustache_config&& other) :
    partialsDirs(std::move(other.partialsDirs)) { }

    mustache_config& operator=(mustache_config&& other) {
        this->partialsDirs = std::move(other.partialsDirs);
        return *this;
    }

    mustache_config() { }

    mustache_config(const staticlib::serialization::json_value& json) {
        namespace ss = staticlib::serialization;
        for (const ss::json_field& fi : json.as_object()) {
            auto& name = fi.name();
            if ("partialsDirs" == name) {
                for (const ss::json_value& va : fi.as_array_or_throw(name)) {
                    if (ss::json_type::string != va.type() || va.as_string().empty()) {
                        throw common::wilton_internal_exception(TRACEMSG(
                                "Invalid 'mustache.partialsDirs.el' value,"
                                " type: [" + ss::stringify_json_type(va.type()) + "]," +
                                " value: [" + ss::dump_json_to_string(va) + "]"));
                    }
                    partialsDirs.emplace_back(va.as_string());
                }
            } else {
                throw common::wilton_internal_exception(TRACEMSG("Unknown 'mustache' field: [" + name + "]"));
            }
        }
    }

    staticlib::serialization::json_value to_json() const {
        namespace sr = staticlib::ranges;
        namespace ss = staticlib::serialization;
        return {
            { "partialsDirs", [this]{
                auto ra = sr::transform(sr::refwrap(partialsDirs), [this](const std::string& el) {
                    return ss::json_value(el);
                });
                return ra.to_vector();
            }() }
        };
    }
};

} // namespace
}

#endif	/* WILTON_SERVERCONF_MUSTACHE_CONFIG_HPP */


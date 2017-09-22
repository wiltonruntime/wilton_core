/* 
 * File:   mustache_config.hpp
 * Author: alex
 *
 * Created on September 7, 2016, 11:22 AM
 */

#ifndef WILTON_SERVERCONF_MUSTACHE_CONFIG_HPP
#define WILTON_SERVERCONF_MUSTACHE_CONFIG_HPP

#include <string>

#include "staticlib/json.hpp"

#include "wilton/support/exception.hpp"

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

    mustache_config(const sl::json::value& json) {
        for (const sl::json::field& fi : json.as_object()) {
            auto& name = fi.name();
            if ("partialsDirs" == name) {
                for (const sl::json::value& va : fi.as_array_or_throw(name)) {
                    if (sl::json::type::string != va.json_type() || va.as_string().empty()) {
                        throw support::exception(TRACEMSG(
                                "Invalid 'mustache.partialsDirs.el' value,"
                                " type: [" + sl::json::stringify_json_type(va.json_type()) + "]," +
                                " value: [" + va.dumps() + "]"));
                    }
                    partialsDirs.emplace_back(va.as_string());
                }
            } else {
                throw support::exception(TRACEMSG("Unknown 'mustache' field: [" + name + "]"));
            }
        }
    }

    sl::json::value to_json() const {
        return {
            { "partialsDirs", [this]{
                auto ra = sl::ranges::transform(partialsDirs, [this](const std::string& el) {
                    return sl::json::value(el);
                });
                return ra.to_vector();
            }() }
        };
    }
};

} // namespace
}

#endif /* WILTON_SERVERCONF_MUSTACHE_CONFIG_HPP */


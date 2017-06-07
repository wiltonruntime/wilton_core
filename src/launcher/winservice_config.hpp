/* 
 * File:   winservice_config.hpp
 * Author: alex
 *
 * Created on June 7, 2017, 8:36 AM
 */

#ifndef WILTON_WINSERVICE_CONFIG_HPP
#define	WILTON_WINSERVICE_CONFIG_HPP

#include <cstdint>
#include <string>

#include "staticlib/json.hpp"

#include "common/wilton_internal_exception.hpp"

namespace wilton {
namespace launcher {

class winservice_config {
    std::string service_name;
    std::string display_name;
    std::string user;
    std::string password;

    winservice_config(const winservice_config&) = delete;

    winservice_config& operator=(const winservice_config&) = delete;

    winservice_config(winservice_config&& other) :
    service_name(std::move(other.service_name)),
    display_name(std::move(other.display_name)),
    user(std::move(other.user)),
    password(std::move(other.password)) { }

    winservice_config& operator=(winservice_config&& other) {
        this->service_name = std::move(other.service_name);
        this->display_name = std::move(other.display_name);
        this->user = std::move(other.user);
        this->password = std::move(other.password);
        return *this;
    }

    winservice_config() { }

    winservice_config(const sl::json::value& json) {
        for (const sl::json::field& fi : json.as_object()) {
            auto& name = fi.name();
            if ("serviceName" == name) {
                this->service_name = fi.as_string_nonempty_or_throw(name);
            } else if ("displayName" == name) {
                this->display_name = fi.as_string_nonempty_or_throw(name);
            } else if ("user" == name) {
                this->user = fi.as_string_nonempty_or_throw(name);
            } else if ("password" == name) {
                this->password = fi.as_string_or_throw(name);
            } else {
                throw common::wilton_internal_exception(TRACEMSG("Unknown 'winservice' field: [" + name + "]"));
            }
        }
        if (0 == service_name.length()) throw common::wilton_internal_exception(TRACEMSG(
                "Invalid 'winservice.serviceName' field: []"));
        if (0 == display_name.length()) throw common::wilton_internal_exception(TRACEMSG(
                "Invalid 'winservice.displayName' field: []"));
        if (0 == user.length()) throw common::wilton_internal_exception(TRACEMSG(
                "Invalid 'winservice.user' field: []"));        
    }

    sl::json::field to_json() const {
        return {
            { "serviceName", service_name },
            { "displayName", display_name },
            { "user", user },
            { "password", password }
        };
    }
};

} // namespace
}

#endif	/* WILTON_WINSERVICE_CONFIG_HPP */


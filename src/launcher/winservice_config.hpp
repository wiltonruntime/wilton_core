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

#include "winservice_exception.hpp"

namespace wilton {
namespace launcher {

class winservice_config {
public:
    sl::json::value json_config;
    std::string service_name;
    std::string display_name;
    std::string user;
    std::string password;
    std::string startup_module;

    winservice_config(const winservice_config&) = delete;

    winservice_config& operator=(const winservice_config&) = delete;

    winservice_config(winservice_config&& other) :
    json_config(std::move(other.json_config)),
    service_name(std::move(other.service_name)),
    display_name(std::move(other.display_name)),
    user(std::move(other.user)),
    password(std::move(other.password)),
    startup_module(std::move(other.startup_module)) { }

    winservice_config& operator=(winservice_config&& other) {
        this->json_config = std::move(other.json_config);
        this->service_name = std::move(other.service_name);
        this->display_name = std::move(other.display_name);
        this->user = std::move(other.user);
        this->password = std::move(other.password);
        this->startup_module = std::move(other.startup_module);
        return *this;
    }

    winservice_config() { }

    winservice_config(sl::json::value&& json) :
    json_config(std::move(json)) {
        for (const sl::json::field& fi : json_config["winservice"].as_object()) {
            auto& name = fi.name();
            if ("serviceName" == name) {
                this->service_name = fi.as_string_nonempty_or_throw(name);
            } else if ("displayName" == name) {
                this->display_name = fi.as_string_nonempty_or_throw(name);
            } else if ("user" == name) {
                this->user = fi.as_string_nonempty_or_throw(name);
            } else if ("password" == name) {
                this->password = fi.as_string_or_throw(name);
            } else if ("startupModule" == name) {
                this->startup_module = fi.as_string_or_throw(name);
            } else {
                throw winservice_exception(TRACEMSG("Unknown 'winservice' field: [" + name + "]"));
            }
        }
        if (0 == service_name.length()) throw winservice_exception(TRACEMSG(
                "Invalid 'winservice.serviceName' field: []"));
        if (0 == display_name.length()) throw winservice_exception(TRACEMSG(
                "Invalid 'winservice.displayName' field: []"));
        if (0 == user.length()) throw winservice_exception(TRACEMSG(
                "Invalid 'winservice.user' field: []"));
        if (0 == startup_module.length()) throw winservice_exception(TRACEMSG(
                "Invalid 'winservice.startupModule' field: []"));        
    }

    sl::json::value to_json() const {
        return {
            { "serviceName", service_name },
            { "displayName", display_name },
            { "user", user },
            { "password", "***" },
            { "startupModule", startup_module },
        };
    }

    winservice_config clone() const {
        auto res = winservice_config();
        res.json_config = json_config.clone();
        res.service_name = std::string(service_name.data(), service_name.length());
        res.display_name = std::string(display_name.data(), display_name.length());
        res.user = std::string(user.data(), user.length());
        res.password = std::string(password.data(), password.length());
        res.startup_module = std::string(startup_module.data(), startup_module.length());
        return res;
    }
};

} // namespace
}

#endif	/* WILTON_WINSERVICE_CONFIG_HPP */


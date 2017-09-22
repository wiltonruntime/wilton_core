/* 
 * File:   ssl_config.hpp
 * Author: alex
 *
 * Created on June 13, 2016, 11:35 AM
 */

#ifndef WILTON_SERVERCONF_SSL_CONFIG_HPP
#define WILTON_SERVERCONF_SSL_CONFIG_HPP

#include <string>

#include "staticlib/json.hpp"

#include "wilton/support/exception.hpp"

namespace wilton {
namespace serverconf {

class ssl_config {
public:
    std::string keyFile = "";
    std::string keyPassword = "";
    std::string verifyFile = "";
    std::string verifySubjectSubstr = "";

    ssl_config(const ssl_config&) = delete;

    ssl_config& operator=(const ssl_config&) = delete;

    ssl_config(ssl_config&& other) :
    keyFile(std::move(other.keyFile)),
    keyPassword(std::move(other.keyPassword)),
    verifyFile(std::move(other.verifyFile)),
    verifySubjectSubstr(std::move(other.verifySubjectSubstr)) { }

    ssl_config& operator=(ssl_config&& other) {
        this->keyFile = std::move(other.keyFile);
        this->keyPassword = std::move(other.keyPassword);
        this->verifyFile = std::move(other.verifyFile);
        this->verifySubjectSubstr = std::move(other.verifySubjectSubstr);
        return *this;
    }
    
    ssl_config() { }
    
    ssl_config(const sl::json::value& json) {
        for (const sl::json::field& fi : json.as_object()) {
            auto& name = fi.name();
            if ("keyFile" == name) {
                this->keyFile = fi.as_string_nonempty_or_throw(name);
            } else if ("keyPassword" == name) {
                // empty string allowed
                this->keyPassword = fi.as_string();
            } else if ("verifyFile" == name) {
                this->verifyFile = fi.as_string_nonempty_or_throw(name);
            } else if ("verifySubjectSubstr" == name) {
                this->verifySubjectSubstr = fi.as_string_nonempty_or_throw(name);
            } else {
                throw support::exception(TRACEMSG("Unknown 'ssl' field: [" + name + "]"));                
            }
        }
    }
    
    sl::json::value to_json() const {
        return {
            { "keyFile", keyFile },
            { "keyPassword", keyPassword},
            { "verifyFile", verifyFile},
            { "verifySubjectSubstr", verifySubjectSubstr}
        };
    }
    
    bool is_empty() const {
        return keyFile.empty();
    }
    
};

} // namespace
}

#endif /* WILTON_SERVERCONF_SSL_CONFIG_HPP */


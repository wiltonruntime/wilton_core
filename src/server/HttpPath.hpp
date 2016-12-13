/* 
 * File:   HttpPath.hpp
 * Author: alex
 *
 * Created on December 10, 2016, 9:55 PM
 */

#ifndef WILTON_SERVER_HTTPPATH_HPP
#define	WILTON_SERVER_HTTPPATH_HPP

#include <cstdint>
#include <string>
#include <functional>

#include "wilton/wilton.h"

#include "staticlib/config.hpp"
#include "staticlib/utils.hpp"

#include "common/WiltonInternalException.hpp"
#include "server/Request.hpp"



namespace wilton {
namespace server {

namespace { // anonymous

namespace sc = staticlib::config;
namespace su = staticlib::utils;

} // namespace

class HttpPath {
public:
    std::string method;
    std::string path;
    std::function<void(Request&)> handler;

    HttpPath(std::string method, std::string path, std::function<void(Request&)> handler) :
    method(std::move(method)),
    path(std::move(path)),
    handler(handler) { }    
    
    HttpPath(const HttpPath&) = delete;
    
    HttpPath& operator=(const HttpPath&) = delete;
    
    HttpPath(HttpPath&& other) :
    method(std::move(other.method)),
    path(std::move(other.path)),
    handler(other.handler) { }
    
    HttpPath& operator=(HttpPath&&) = delete;
};

} // namespace
}

#endif	/* WILTON_SERVER_HTTPPATH_HPP */


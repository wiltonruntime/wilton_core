/* 
 * File:   http_path.hpp
 * Author: alex
 *
 * Created on December 10, 2016, 9:55 PM
 */

#ifndef WILTON_SERVER_HTTP_PATH_HPP
#define WILTON_SERVER_HTTP_PATH_HPP

#include <cstdint>
#include <string>
#include <functional>

#include "wilton/wilton.h"

#include "staticlib/config.hpp"
#include "staticlib/utils.hpp"

#include "wilton/support/exception.hpp"

#include "server/request.hpp"



namespace wilton {
namespace server {

namespace { // anonymous

namespace sc = staticlib::config;
namespace su = staticlib::utils;

} // namespace

class http_path {
public:
    std::string method;
    std::string path;
    std::function<void(request&)> handler;

    http_path(std::string method, std::string path, std::function<void(request&)> handler) :
    method(std::move(method)),
    path(std::move(path)),
    handler(handler) { }    
    
    http_path(const http_path&) = delete;
    
    http_path& operator=(const http_path&) = delete;
    
    http_path(http_path&& other) :
    method(std::move(other.method)),
    path(std::move(other.path)),
    handler(other.handler) { }
    
    http_path& operator=(http_path&&) = delete;
};

} // namespace
}

#endif /* WILTON_SERVER_HTTP_PATH_HPP */


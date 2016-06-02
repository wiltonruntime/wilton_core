/*
 * Copyright 2015, alex at staticlibs.net
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* 
 * File:   StringPayloadHandler.hpp
 * Author: alex
 *
 * Created on April 5, 2015, 5:49 PM
 */

#ifndef WILTON_C_STRINGPAYLOADHANDLER_HPP
#define	WILTON_C_STRINGPAYLOADHANDLER_HPP

#include <exception>
#include <functional>
#include <memory>
#include <string>

#include "staticlib/httpserver/http_request.hpp"

#include "WiltonInternalException.hpp"

namespace wilton {

namespace { // anonymous

// todo: make configurable
const size_t MAX_LENGTH = 1024 * 1024; // the same as default in pion
const std::string EMPTY_STRING{""};

} // namespace

class StringPayloadHandler {
    std::shared_ptr<std::string> buffer;

public:
    // intrusive copy and move constructors to satisfy std::function
    StringPayloadHandler(const StringPayloadHandler& other) :
    buffer(other.buffer) { }
    
    StringPayloadHandler& operator=(const StringPayloadHandler& other) {
        buffer = other.buffer;
        return *this;
    }
    
    StringPayloadHandler(StringPayloadHandler&& other) :    
    buffer(std::move(other.buffer)) { }
    
    StringPayloadHandler& operator=(StringPayloadHandler&& other) {
        buffer = std::move(other.buffer);
        return *this;
    }
    
    StringPayloadHandler() : 
    buffer(new std::string()) { }

    static StringPayloadHandler create(staticlib::httpserver::http_request_ptr& /* request */) {
        return StringPayloadHandler{};
    }

    static const std::string& get_payload(staticlib::httpserver::http_request_ptr& request) {
        auto ph = request->get_payload_handler<StringPayloadHandler>();
        if (ph) {
            return ph->get_buffer();
        } else {
            return EMPTY_STRING;
        }
    }

    void operator()(const char* s, size_t n) {
        if (buffer->length() + n < MAX_LENGTH) {
            buffer->append(s, n);
        } else {
            throw WiltonInternalException("Request body exceeds 1MB, client connection will be closed");
        }
    }

    std::string& get_buffer() {        
        return *buffer;
    }

};

} // namespace

#endif	/* WILTON_C_STRINGPAYLOADHANDLER_HPP */

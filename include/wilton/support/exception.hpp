/*
 * Copyright 2017, alex at staticlibs.net
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
 * File:   wilton_support_exception.hpp
 * Author: alex
 *
 * Created on June 12, 2017, 5:01 PM
 */

#ifndef WILTON_SUPPORT_EXCEPTION_HPP
#define WILTON_SUPPORT_EXCEPTION_HPP

#include <string>

#include "staticlib/support/exception.hpp"

#include "wilton/wilton.h"

namespace wilton {
namespace support {

/**
 * Module specific exception
 */
class exception : public sl::support::exception {
public:
    /**
     * Default constructor
     */
    exception() = default;

    /**
     * Constructor with message
     * 
     * @param msg error message
     */
    exception(const std::string& msg) :
    sl::support::exception(msg) { }

};

inline void throw_wilton_error(char* err, const std::string& msg) {
    wilton_free(err);
    throw exception(msg);
}

} //namespace
}

#endif /* WILTON_SUPPORT_EXCEPTION_HPP */


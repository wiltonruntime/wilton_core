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


/* 
 * File:   wilton_support_exception.hpp
 * Author: alex
 *
 * Created on June 12, 2017, 5:01 PM
 */

#ifndef WILTON_SUPPORT_WILTON_SUPPORT_EXCEPTION_HPP
#define	WILTON_SUPPORT_WILTON_SUPPORT_EXCEPTION_HPP

#include <string>

#include "staticlib/support/exception.hpp"

namespace wilton {
namespace support {

/**
 * Module specific exception
 */
class wilton_support_exception : public sl::support::exception {
public:
    /**
     * Default constructor
     */
    wilton_support_exception() = default;

    /**
     * Constructor with message
     * 
     * @param msg error message
     */
    wilton_support_exception(const std::string& msg) :
    sl::support::exception(msg) { }

};

} //namespace
}

#endif	/* WILTON_SUPPORT_WILTON_SUPPORT_EXCEPTION_HPP */


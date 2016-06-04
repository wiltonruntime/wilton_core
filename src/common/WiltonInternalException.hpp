/* 
 * File:   WiltonInternalException.hpp
 * Author: alex
 *
 * Created on May 5, 2016, 7:35 PM
 */

#ifndef WILTON_COMMON_WILTONINTERNALEXCEPTION_HPP
#define	WILTON_COMMON_WILTONINTERNALEXCEPTION_HPP

#include <string>

#include "staticlib/config/BaseException.hpp"

namespace wilton {
namespace common {

/**
 * Module specific exception
 */
class WiltonInternalException : public staticlib::config::BaseException {
public:
    /**
     * Default constructor
     */
    WiltonInternalException() = default;

    /**
     * Constructor with message
     * 
     * @param msg error message
     */
    WiltonInternalException(const std::string& msg) :
    staticlib::config::BaseException(msg) { }

};

} //namespace
}

#endif	/* WILTON_COMMON_WILTONINTERNALEXCEPTION_HPP */


/* 
 * File:   WiltonInternalException.hpp
 * Author: alex
 *
 * Created on May 5, 2016, 7:35 PM
 */

#ifndef WILTONINTERNALEXCEPTION_HPP
#define	WILTONINTERNALEXCEPTION_HPP

#include <string>

#include "staticlib/config/BaseException.hpp"

namespace wilton {

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

#endif	/* WILTONINTERNALEXCEPTION_HPP */


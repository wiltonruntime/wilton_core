/* 
 * File:   winservice_exception.hpp
 * Author: alex
 *
 * Created on May 5, 2016, 7:35 PM
 */

#ifndef WILTON_LAUNCHER_WINSERVICE_EXCEPTION_HPP
#define	WILTON_LAUNCHER_WINSERVICE_EXCEPTION_HPP

#include <string>

#include "staticlib/support/exception.hpp"

namespace wilton {
namespace launcher {

/**
 * Module specific exception
 */
class winservice_exception : public sl::support::exception {
public:
    /**
     * Default constructor
     */
    winservice_exception() = default;

    /**
     * Constructor with message
     * 
     * @param msg error message
     */
    winservice_exception(const std::string& msg) :
    sl::support::exception(msg) { }

};

} //namespace
}

#endif	/* WILTON_LAUNCHER_WINSERVICE_EXCEPTION_HPP */


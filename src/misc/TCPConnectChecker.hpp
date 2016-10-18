/* 
 * File:   TCPConnectChecker.hpp
 * Author: alex
 *
 * Created on October 18, 2016, 12:21 PM
 */

#ifndef WILTON_MISC_TCPCONNECTCHECKER_HPP
#define	WILTON_MISC_TCPCONNECTCHECKER_HPP

#include <cstdint>
#include <chrono>
#include <string>

#include "staticlib/pimpl.hpp"

#include "common/WiltonInternalException.hpp"

namespace wilton {
namespace misc {

class TCPConnectChecker : public staticlib::pimpl::PimplObject {
protected:
    /**
     * Implementation class
     */
    class Impl;

public:
    /**
     * PIMPL-specific constructor
     * 
     * @param pimpl impl object
     */
    PIMPL_CONSTRUCTOR(TCPConnectChecker)

    static std::string wait_for_connection(std::chrono::milliseconds timeout, 
            const std::string& ip_addr, uint16_t tcp_port);

};

} // namespace
}

#endif	/* WILTON_MISC_TCPCONNECTCHECKER_HPP */


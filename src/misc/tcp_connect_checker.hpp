/* 
 * File:   tcp_connect_checker.hpp
 * Author: alex
 *
 * Created on October 18, 2016, 12:21 PM
 */

#ifndef WILTON_MISC_TCP_CONNECT_CHECKER_HPP
#define	WILTON_MISC_TCP_CONNECT_CHECKER_HPP

#include <cstdint>
#include <chrono>
#include <string>

#include "staticlib/pimpl.hpp"

#include "common/wilton_internal_exception.hpp"

namespace wilton {
namespace misc {

class tcp_connect_checker : public sl::pimpl::object {
protected:
    /**
     * implementation class
     */
    class impl;

public:
    /**
     * PIMPL-specific constructor
     * 
     * @param pimpl impl object
     */
    PIMPL_CONSTRUCTOR(tcp_connect_checker)

    static std::string wait_for_connection(std::chrono::milliseconds timeout, 
            const std::string& ip_addr, uint16_t tcp_port);

};

} // namespace
}

#endif	/* WILTON_MISC_TCP_CONNECT_CHECKER_HPP */


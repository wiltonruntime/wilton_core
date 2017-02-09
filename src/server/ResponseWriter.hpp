/* 
 * File:   ResponseWriter.hpp
 * Author: alex
 *
 * Created on June 19, 2016, 9:43 PM
 */

#ifndef WILTON_SERVER_RESPONSEWRITER_HPP
#define	WILTON_SERVER_RESPONSEWRITER_HPP

#include <cstdint>

#include "staticlib/pimpl.hpp"

#include "common/WiltonInternalException.hpp"

namespace wilton {
namespace server {

class ResponseWriter : public staticlib::pimpl::pimpl_object {
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
    PIMPL_CONSTRUCTOR(ResponseWriter)

    void send(const char* data, uint32_t data_len);
  
    // private api
    ResponseWriter(void* /* staticlib::httpserver::http_response_writer_ptr&& */ writer);
};

} // namespace
}

#endif	/* WILTON_SERVER_RESPONSEWRITER_HPP */


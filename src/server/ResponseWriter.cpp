/* 
 * File:   ResponseWriter.cpp
 * Author: alex
 * 
 * Created on June 19, 2016, 9:49 PM
 */

#include "server/ResponseWriter.hpp"

#include "staticlib/httpserver/http_response_writer.hpp"
#include "staticlib/pimpl/pimpl_forward_macros.hpp"

namespace wilton {
namespace server {

namespace { // anonymous

namespace sh = staticlib::httpserver;

} //namespace

class ResponseWriter::Impl : public staticlib::pimpl::PimplObject::Impl {
    sh::http_response_writer_ptr writer;

public:
    Impl(void* /* sh::http_response_writer_ptr* */ writer) :
    writer(std::move(*static_cast<sh::http_response_writer_ptr*> (writer))) { }    
    
    void send(ResponseWriter&, const char* data, uint32_t data_len) {
        writer->write(data, data_len);
        writer->send();
    }
    
};
PIMPL_FORWARD_CONSTRUCTOR(ResponseWriter, (void*), (), common::WiltonInternalException)
PIMPL_FORWARD_METHOD(ResponseWriter, void, send, (const char*)(uint32_t), (), common::WiltonInternalException)

} // namespace
}

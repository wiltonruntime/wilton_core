/* 
 * File:   response_writer.cpp
 * Author: alex
 * 
 * Created on June 19, 2016, 9:49 PM
 */

#include "server/response_writer.hpp"

#include "staticlib/httpserver/http_response_writer.hpp"
#include "staticlib/pimpl/pimpl_forward_macros.hpp"

namespace wilton {
namespace server {

namespace { // anonymous

namespace sh = staticlib::httpserver;

} //namespace

class response_writer::impl : public staticlib::pimpl::pimpl_object::impl {
    sh::http_response_writer_ptr writer;

public:
    impl(void* /* sh::http_response_writer_ptr* */ writer) :
    writer(std::move(*static_cast<sh::http_response_writer_ptr*> (writer))) { }    
    
    void send(response_writer&, const char* data, uint32_t data_len) {
        writer->write(data, data_len);
        writer->send();
    }
    
};
PIMPL_FORWARD_CONSTRUCTOR(response_writer, (void*), (), common::wilton_internal_exception)
PIMPL_FORWARD_METHOD(response_writer, void, send, (const char*)(uint32_t), (), common::wilton_internal_exception)

} // namespace
}

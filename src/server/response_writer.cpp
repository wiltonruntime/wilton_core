/* 
 * File:   response_writer.cpp
 * Author: alex
 * 
 * Created on June 19, 2016, 9:49 PM
 */

#include "server/response_writer.hpp"

#include "staticlib/pion.hpp"
#include "staticlib/pimpl/forward_macros.hpp"

namespace wilton {
namespace server {

class response_writer::impl : public staticlib::pimpl::object::impl {
    sl::pion::http_response_writer_ptr writer;

public:
    impl(void* /* sl::pion::http_response_writer_ptr* */ writer) :
    writer(std::move(*static_cast<sl::pion::http_response_writer_ptr*> (writer))) { }    
    
    void send(response_writer&, const char* data, uint32_t data_len) {
        writer->write(data, data_len);
        writer->send();
    }
    
};
PIMPL_FORWARD_CONSTRUCTOR(response_writer, (void*), (), support::exception)
PIMPL_FORWARD_METHOD(response_writer, void, send, (const char*)(uint32_t), (), support::exception)

} // namespace
}

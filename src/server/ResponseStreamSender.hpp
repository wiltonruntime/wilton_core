/*
 * Copyright 2016, alex at staticlibs.net
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* 
 * File:   ResponseStreamSender.hpp
 * Author: alex
 *
 * Created on April 5, 2015, 10:58 PM
 */

#ifndef WILTON_SERVER_RESPONSESTREAMSENDER_HPP
#define	WILTON_SERVER_RESPONSESTREAMSENDER_HPP

#include <memory>
#include <streambuf>

#include "asio.hpp"

#include "staticlib/httpserver/http_response_writer.hpp"
#include "staticlib/httpserver/tcp_connection.hpp"

#include "staticlib/io/operations.hpp"
#include "staticlib/io/streambuf_source.hpp"


namespace wilton {
namespace server {

class ResponseStreamSender : public std::enable_shared_from_this<ResponseStreamSender> {
    staticlib::httpserver::http_response_writer_ptr writer;
    std::unique_ptr<std::streambuf> stream;
    std::function<void(bool)> finalizer;

    std::array<char, 4096> buf;

public:
    ResponseStreamSender(staticlib::httpserver::http_response_writer_ptr writer, 
            std::unique_ptr<std::streambuf>&& stream, 
            std::function<void(bool)> finalizer = [](bool){}) :
    writer(std::move(writer)),
    stream(std::move(stream)),
    finalizer(std::move(finalizer)) { }

    void send() {
        asio::error_code ec{};
        handle_write(ec, 0);
    }

    void handle_write(const asio::error_code& ec, size_t /* bytes_written */) {
        namespace si = staticlib::io;
        if (!ec) {
            auto src = si::streambuf_source(stream.get());
            size_t len = si::read_all(src, buf);
            writer->clear();
            if (len > 0) {
                if (buf.size() == len) {
                    writer->write_no_copy(buf.data(), len);
                    writer->send_chunk(std::bind(&ResponseStreamSender::handle_write, shared_from_this(),
                            std::placeholders::_1, std::placeholders::_2));
                } else {
                    writer->write(buf.data(), len);
                    writer->send_final_chunk();
                    finalizer(true);
                }
            } else {
                writer->send_final_chunk();
                finalizer(true);
            }
        } else {
            // make sure it will get closed
            writer->get_connection()->set_lifecycle(staticlib::httpserver::tcp_connection::LIFECYCLE_CLOSE);
            finalizer(false);
        }
    }

};

} // namespace
}

#endif	/* WILTON_SERVER_RESPONSESTREAMSENDER_HPP */

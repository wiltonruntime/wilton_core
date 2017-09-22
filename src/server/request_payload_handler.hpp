/*
 * Copyright 2015, alex at staticlibs.net
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
 * File:   request_payload_handler.hpp
 * Author: alex
 *
 * Created on April 5, 2015, 5:49 PM
 */

#ifndef WILTON_SERVER_REQUEST_PAYLOAD_HANDLER_HPP
#define WILTON_SERVER_REQUEST_PAYLOAD_HANDLER_HPP

#include <cstdint>
#include <cstdio>
#include <array>
#include <exception>
#include <functional>
#include <memory>
#include <string>

#include "staticlib/pion.hpp"
#include "staticlib/config.hpp"
#include "staticlib/io.hpp"
#include "staticlib/tinydir.hpp"
#include "staticlib/utils.hpp"

#include "wilton/support/exception.hpp"

#include "serverconf/request_payload_config.hpp"

namespace wilton {
namespace server {

class request_payload_handler {
    enum class payload_state {
        memory, file
    };

    class payload_data {
        friend class request_payload_handler;
        serverconf::request_payload_config conf;
        uint64_t counter = 0;
        std::string buffer = "";
        std::string filename;
        std::unique_ptr<sl::tinydir::file_sink> file;
        payload_state state = payload_state::memory;
        std::unique_ptr<sl::utils::random_string_generator> rng;

    public:        
        payload_data(const serverconf::request_payload_config& conf) :
        conf(conf.clone()) { }
      
        sl::utils::random_string_generator& randomgen() {
            if (nullptr == rng.get()) {
                rng.reset(new sl::utils::random_string_generator());
            }
            return *rng;
        }
        
        payload_data(const payload_data&) = delete;
        payload_data& operator=(const payload_data&) = delete;
    };
    
    std::shared_ptr<payload_data> data;

public:
    request_payload_handler(const request_payload_handler& other) :
    data(other.data) { }
    
    request_payload_handler& operator=(const request_payload_handler& other) {
        data = other.data;
        return *this;
    }
    
    request_payload_handler(request_payload_handler&& other) :    
    data(std::move(other.data)) { }
    
    request_payload_handler& operator=(request_payload_handler&& other) {
        data = std::move(other.data);
        return *this;
    }
    
    ~request_payload_handler() STATICLIB_NOEXCEPT {
        try {
            if (nullptr != data.get() && !data->filename.empty()) {
                close_file_writer();
                std::remove(data->filename.c_str());
            }
        } catch(...) {
            // ignore
        }
    }
    
    request_payload_handler(const serverconf::request_payload_config& conf) : 
    data(std::make_shared<payload_data>(conf)) { }

    static const std::string& get_data_string(sl::pion::http_request_ptr& request) {
        auto ph = request->get_payload_handler<request_payload_handler>();
        if (!ph) throw support::exception(TRACEMSG("System error in payload handler access"));
        return ph->get_data_as_string();
    }

    static const std::string& get_data_filename(sl::pion::http_request_ptr& request) {
        auto ph = request->get_payload_handler<request_payload_handler>();
        if (!ph) throw support::exception(TRACEMSG("System error in payload handler access"));
        return ph->get_data_as_filename();
    }
    
    void operator()(const char* s, size_t n) {
        switch (data->state) {
        case payload_state::memory:
            if (data->buffer.length() + n < data->conf.memoryLimitBytes) {
                data->buffer.append(s, n);
                return;
            } else if (data->conf.tmpDirPath.empty()) {
                throw support::exception(TRACEMSG("Request body exceeds" +
                        " limit (bytes): [" + sl::support::to_string(data->conf.memoryLimitBytes) + "]"));
            } else {
                data->state = payload_state::file;
                data->filename = gen_filename();
                data->file = std::unique_ptr<sl::tinydir::file_sink>(new sl::tinydir::file_sink(data->filename));
                if (!data->buffer.empty()) {
                    sl::io::write_all(*data->file, data->buffer);
                    data->buffer = "";
                }
            }
            // fall through
        case payload_state::file:
            if (nullptr != data->file.get()) {
                sl::io::write_all(*data->file, {s, n});
            } else throw support::exception(TRACEMSG("Invalid payload handler data state"));
            return;
        default:
            throw support::exception(TRACEMSG("Invalid payload handler state"));
        }
    }

private:        
    payload_data& get_data() {        
        return *data;
    }
    
    std::string gen_filename() {
        return data->conf.tmpDirPath + "/" + sl::support::to_string(data->counter++) + "_" +
                data->randomgen().generate(data->conf.tmpFilenameLength);
    }
    
    void close_file_writer() {
        data->file.reset(nullptr);
    }
    
    const std::string& get_data_as_string() {
        close_file_writer();
        switch (data->state) {
        case payload_state::file: {
            data->state = payload_state::memory;
            auto fd = sl::tinydir::file_source(data->filename);
            sl::io::string_sink sink;
            std::array<char, 4096> buf;
            sl::io::copy_all(fd, sink, buf);
            data->buffer = std::move(sink.get_string());            
        } // fall through     
        case payload_state::memory:
            return data->buffer;               
        default:
            throw support::exception(TRACEMSG("Invalid payload handler state"));
        }
    }
    
    const std::string& get_data_as_filename() {
        close_file_writer();
        switch (data->state) {
        case payload_state::memory:
            data->state = payload_state::file;
            if (data->filename.empty()) {
                data->filename = gen_filename();
                auto fd = sl::tinydir::file_sink(data->filename);
                sl::io::write_all(fd, data->buffer);
            }
            // fall through
        case payload_state::file:
            return data->filename;
        default:
            throw support::exception(TRACEMSG("Invalid payload handler state"));
        }
    }

};

} // namespace
}

#endif /* WILTON_SERVER_REQUEST_PAYLOAD_HANDLER_HPP */

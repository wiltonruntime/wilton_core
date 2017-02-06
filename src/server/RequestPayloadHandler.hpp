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
 * File:   RequestPayloadHandler.hpp
 * Author: alex
 *
 * Created on April 5, 2015, 5:49 PM
 */

#ifndef WILTON_SERVER_REQUESTPAYLOADHANDLER_HPP
#define	WILTON_SERVER_REQUESTPAYLOADHANDLER_HPP

#include <cstdint>
#include <cstdio>
#include <array>
#include <exception>
#include <functional>
#include <memory>
#include <string>

#include "staticlib/httpserver/http_request.hpp"
#include "staticlib/config.hpp"
#include "staticlib/io.hpp"
#include "staticlib/tinydir.hpp"
#include "staticlib/utils.hpp"

#include "common/WiltonInternalException.hpp"
#include "serverconf/RequestPayloadConfig.hpp"

namespace wilton {
namespace server {

namespace { // anonymous

namespace sc = staticlib::config;
namespace si = staticlib::io;
namespace st = staticlib::tinydir;
namespace su = staticlib::utils;

} // namespace

class RequestPayloadHandler {
    enum class State {
        MEMORY, FILE
    };

    class Data {
        friend class RequestPayloadHandler;
        serverconf::RequestPayloadConfig conf;
        uint64_t counter = 0;
        std::string buffer = "";
        std::string filename;
        std::unique_ptr<st::TinydirFileSink> file;
        State state = State::MEMORY;
        su::RandomStringGenerator rng;

    public:        
        Data(const serverconf::RequestPayloadConfig& conf) :
        conf(conf.clone()) { }
        
        Data(const Data&) = delete;
        Data& operator=(const Data&) = delete;
    };
    
    std::shared_ptr<Data> data;

public:
    RequestPayloadHandler(const RequestPayloadHandler& other) :
    data(other.data) { }
    
    RequestPayloadHandler& operator=(const RequestPayloadHandler& other) {
        data = other.data;
        return *this;
    }
    
    RequestPayloadHandler(RequestPayloadHandler&& other) :    
    data(std::move(other.data)) { }
    
    RequestPayloadHandler& operator=(RequestPayloadHandler&& other) {
        data = std::move(other.data);
        return *this;
    }
    
    ~RequestPayloadHandler() STATICLIB_NOEXCEPT {
        try {
            if (nullptr != data.get() && !data->filename.empty()) {
                close_file_writer();
                std::remove(data->filename.c_str());
            }
        } catch(...) {
            // ignore
        }
    }
    
    RequestPayloadHandler(const serverconf::RequestPayloadConfig& conf) : 
    data(std::make_shared<Data>(conf)) { }

    static const std::string& get_data_string(staticlib::httpserver::http_request_ptr& request) {
        auto ph = request->get_payload_handler<RequestPayloadHandler>();
        if (!ph) throw common::WiltonInternalException(TRACEMSG("System error in payload handler access"));
        return ph->get_data_as_string();
    }

    static const std::string& get_data_filename(staticlib::httpserver::http_request_ptr& request) {
        auto ph = request->get_payload_handler<RequestPayloadHandler>();
        if (!ph) throw common::WiltonInternalException(TRACEMSG("System error in payload handler access"));
        return ph->get_data_as_filename();
    }
    
    void operator()(const char* s, size_t n) {
        switch (data->state) {
        case State::MEMORY:
            if (data->buffer.length() + n < data->conf.memoryLimitBytes) {
                data->buffer.append(s, n);
                return;
            } else if (data->conf.tmpDirPath.empty()) {
                throw common::WiltonInternalException(TRACEMSG("Request body exceeds" +
                        " limit (bytes): [" + sc::to_string(data->conf.memoryLimitBytes) + "]"));
            } else {
                data->state = State::FILE;
                data->filename = gen_filename();
                data->file = std::unique_ptr<st::TinydirFileSink>(new st::TinydirFileSink(data->filename));
                if (!data->buffer.empty()) {
                    si::write_all(*data->file, data->buffer);
                    data->buffer = "";
                }
            }
            // fall through
        case State::FILE:
            if (nullptr != data->file.get()) {
                si::write_all(*data->file, {s, n});
            } else throw common::WiltonInternalException(TRACEMSG("Invalid payload handler data state"));
            return;
        default:
            throw common::WiltonInternalException(TRACEMSG("Invalid payload handler state"));
        }
    }

private:        
    Data& get_data() {        
        return *data;
    }
    
    std::string gen_filename() {
        return data->conf.tmpDirPath + "/" + sc::to_string(data->counter++) + "_" +
                data->rng.generate(data->conf.tmpFilenameLength);
    }
    
    void close_file_writer() {
        data->file.reset(nullptr);
    }
    
    const std::string& get_data_as_string() {
        close_file_writer();
        switch (data->state) {
        case State::FILE: {
            data->state = State::MEMORY;
            auto fd = st::TinydirFileSource(data->filename);
            si::string_sink sink;
            std::array<char, 4096> buf;
            si::copy_all(fd, sink, buf);
            data->buffer = std::move(sink.get_string());            
        } // fall through     
        case State::MEMORY:
            return data->buffer;               
        default:
            throw common::WiltonInternalException(TRACEMSG("Invalid payload handler state"));
        }
    }
    
    const std::string& get_data_as_filename() {
        close_file_writer();
        switch (data->state) {
        case State::MEMORY:
            data->state = State::FILE;
            if (data->filename.empty()) {
                data->filename = gen_filename();
                auto fd = st::TinydirFileSink(data->filename);
                si::write_all(fd, data->buffer);
            }
            // fall through
        case State::FILE:
            return data->filename;
        default:
            throw common::WiltonInternalException(TRACEMSG("Invalid payload handler state"));
        }
    }

};

} // namespace
}

#endif	/* WILTON_SERVER_REQUESTPAYLOADHANDLER_HPP */

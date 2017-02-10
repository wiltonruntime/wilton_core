/* 
 * File:   wilton_server.cpp
 * Author: alex
 * 
 * Created on June 4, 2016, 8:15 PM
 */

#include "wilton/wilton.h"

#include <functional>
#include <vector>

#include "staticlib/config.hpp"
#include "staticlib/serialization.hpp"
#include "staticlib/utils.hpp"

#include "server/http_path.hpp"
#include "server/request.hpp"
#include "server/response_writer.hpp"
#include "server/server.hpp"

#include "serverconf/response_metadata.hpp"

namespace { // anonymous

namespace sc = staticlib::config;
namespace ss = staticlib::serialization;
namespace su = staticlib::utils;
namespace ws = wilton::server;
namespace wj = wilton::serverconf;

} // namespace

struct wilton_Server {
private:
    ws::server delegate;

public:
    wilton_Server(ws::server&& delegate) :
    delegate(std::move(delegate)) { }

    ws::server& impl() {
        return delegate;
    }
};

struct wilton_Request {
private:
    sc::observer_ptr<wilton::server::request> delegate;
    
public:
    wilton_Request(wilton::server::request& delegate) :
    delegate(std::addressof(delegate)) { }

    wilton::server::request& impl() {
        return *delegate;
    }
};

struct wilton_ResponseWriter {
private:
    ws::response_writer delegate;

public:
    wilton_ResponseWriter(ws::response_writer&& delegate) :
    delegate(std::move(delegate)) { }

    ws::response_writer& impl() {
        return delegate;
    }
};

struct wilton_HttpPath {
private:
    ws::http_path delegate;

public:

    wilton_HttpPath(ws::http_path&& delegate) :
    delegate(std::move(delegate)) { }

    ws::http_path& impl() {
        return delegate;
    }
};

namespace { // anonymous

std::vector<sc::observer_ptr<ws::http_path>> wrap_paths(wilton_HttpPath** paths, uint16_t paths_len) {
    std::vector<sc::observer_ptr<ws::http_path>> res;
    for (int i = 0; i < paths_len; i++) {
        wilton_HttpPath* ptr = paths[i];
        auto obs = sc::make_observer_ptr(ptr->impl());
        res.push_back(obs);
    }
    return res;
}

} // namespace

char* wilton_HttpPath_create(wilton_HttpPath** http_path_out, const char* method, int method_len,
        const char* path, int path_len, void* handler_ctx, 
        void (*handler_cb)(void* handler_ctx, wilton_Request* request)) {
    if (nullptr == http_path_out) return su::alloc_copy(TRACEMSG("Null 'http_path_out' parameter specified"));
    if (nullptr == method) return su::alloc_copy(TRACEMSG("Null 'method' parameter specified"));
    if (!sc::is_uint16_positive(method_len)) return su::alloc_copy(TRACEMSG(
            "Invalid 'method_len' parameter specified: [" + sc::to_string(method_len) + "]"));
    if (nullptr == path) return su::alloc_copy(TRACEMSG("Null 'path' parameter specified"));
    if (!sc::is_uint16_positive(path_len)) return su::alloc_copy(TRACEMSG(
            "Invalid 'path_len' parameter specified: [" + sc::to_string(path_len) + "]"));
    if (nullptr == handler_cb) return su::alloc_copy(TRACEMSG("Null 'handler_cb' parameter specified"));
    try {
        uint16_t method_len_u16 = static_cast<uint16_t> (method_len);
        std::string method_str = std::string(method, method_len_u16);
        uint16_t path_len_u16 = static_cast<uint16_t> (path_len);
        std::string path_str = std::string(path, path_len_u16);
        auto ha_ctx = handler_ctx;
        auto ha_cb = handler_cb;
        auto handler = [ha_ctx, ha_cb](ws::request& req) {
            wilton_Request req_pass{req};
            ha_cb(ha_ctx, std::addressof(req_pass));
        };
        ws::http_path http_path{std::move(method_str), std::move(path_str), handler};
        wilton_HttpPath* http_path_ptr = new wilton_HttpPath(std::move(http_path));
        *http_path_out = http_path_ptr;
        return nullptr;
    } catch (const std::exception& e) {
        return su::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

char* wilton_HttpPath_destroy(wilton_HttpPath* path) {
    delete path;
    return nullptr;
}

// TODO: fixme json copy
char* wilton_Server_create /* noexcept */ (wilton_Server** server_out, const char* conf_json,
        int conf_json_len, wilton_HttpPath** paths, int paths_len) /* noexcept */ {
    if (nullptr == server_out) return su::alloc_copy(TRACEMSG("Null 'server_out' parameter specified"));    
    if (nullptr == conf_json) return su::alloc_copy(TRACEMSG("Null 'conf_json' parameter specified"));
    if (!sc::is_uint32_positive(conf_json_len)) return su::alloc_copy(TRACEMSG(
            "Invalid 'conf_json_len' parameter specified: [" + sc::to_string(conf_json_len) + "]"));
    if (nullptr == paths) return su::alloc_copy(TRACEMSG("Null 'paths' parameter specified"));
    if (!sc::is_uint16_positive(paths_len)) return su::alloc_copy(TRACEMSG(
            "Invalid 'paths_len' parameter specified: [" + sc::to_string(paths_len) + "]"));
    try {
        uint32_t conf_json_len_u32 = static_cast<uint32_t> (conf_json_len);
        std::string conf_str{conf_json, conf_json_len_u32};                
        ss::json_value json = ss::load_json_from_string(conf_str);
        uint16_t paths_len_u16 = static_cast<uint16_t>(paths_len);
        auto pathsvec = wrap_paths(paths, paths_len_u16);
        ws::server server{std::move(json), std::move(pathsvec)};
        wilton_Server* server_ptr = new wilton_Server(std::move(server));
        *server_out = server_ptr;
        return nullptr;
    } catch (const std::exception& e) {
        return su::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

char* wilton_Server_stop(wilton_Server* server) /* noexcept */ {
    if (nullptr == server) return su::alloc_copy(TRACEMSG("Null 'server' parameter specified"));
    try {
        server->impl().stop();
        delete server;
        return nullptr;
    } catch (const std::exception& e) {
        return su::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

// TODO: fixme json copy
char* wilton_Request_get_request_metadata(wilton_Request* request, char** metadata_json_out,
        int* metadata_json_len_out) /* noexcept */ {
    if (nullptr == request) return su::alloc_copy(TRACEMSG("Null 'server' parameter specified"));
    if (nullptr == metadata_json_out) return su::alloc_copy(TRACEMSG("Null 'metadata_json_out' parameter specified"));
    if (nullptr == metadata_json_len_out) return su::alloc_copy(TRACEMSG("Null 'metadata_json_len_out' parameter specified"));
    try {
        auto meta = request->impl().get_request_metadata();
        ss::json_value json = meta.to_json();
        std::string res = ss::dump_json_to_string(json);
        *metadata_json_out = su::alloc_copy(res);
        *metadata_json_len_out = res.size();
        return nullptr;
    } catch (const std::exception& e) {
        return su::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

// TODO: think about copy
char* wilton_Request_get_request_data(wilton_Request* request, char** data_out,
        int* data_len_out) /* noexcept */ {
    if (nullptr == request) return su::alloc_copy(TRACEMSG("Null 'request' parameter specified"));
    if (nullptr == data_out) return su::alloc_copy(TRACEMSG("Null 'data_out' parameter specified"));
    if (nullptr == data_len_out) return su::alloc_copy(TRACEMSG("Null 'data_len_out' parameter specified"));
    try {
        const std::string& res = request->impl().get_request_data();
        *data_out = su::alloc_copy(res);
        *data_len_out = res.size();
        return nullptr;
    } catch (const std::exception& e) {
        return su::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

WILTON_EXPORT char* wilton_Request_get_request_data_filename(wilton_Request* request, 
        char** filename_out, int* filename_len_out) /* noexcept */ {
    if (nullptr == request) return su::alloc_copy(TRACEMSG("Null 'request' parameter specified"));
    if (nullptr == filename_out) return su::alloc_copy(TRACEMSG("Null 'filename_out' parameter specified"));
    if (nullptr == filename_len_out) return su::alloc_copy(TRACEMSG("Null 'filename_len_out' parameter specified"));
    try {
        const std::string& res = request->impl().get_request_data_filename();
        *filename_out = su::alloc_copy(res);
        *filename_len_out = res.size();
        return nullptr;
    } catch (const std::exception& e) {
        return su::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

// TODO: fixme json copy
char* wilton_Request_set_response_metadata(wilton_Request* request,
        const char* metadata_json, int metadata_json_len) /* noexcept */ {
    if (nullptr == request) return su::alloc_copy(TRACEMSG("Null 'request' parameter specified"));
    if (nullptr == metadata_json) return su::alloc_copy(TRACEMSG("Null 'metadata_json' parameter specified"));
    if (!sc::is_uint32_positive(metadata_json_len)) return su::alloc_copy(TRACEMSG(
            "Invalid 'metadata_json_len' parameter specified: [" + sc::to_string(metadata_json_len) + "]"));
    try {
        uint32_t metadata_json_len_u32 = static_cast<uint32_t> (metadata_json_len);
        std::string metadata{metadata_json, metadata_json_len_u32};
        ss::json_value json = ss::load_json_from_string(metadata);
        wj::response_metadata rm{json};
        request->impl().set_response_metadata(std::move(rm));
        return nullptr;
    } catch (const std::exception& e) {
        return su::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

char* wilton_Request_send_response(wilton_Request* request, const char* data,
        int data_len) /* noexcept */ {
    if (nullptr == request) return su::alloc_copy(TRACEMSG("Null 'request' parameter specified"));
    if (nullptr == data) return su::alloc_copy(TRACEMSG("Null 'data' parameter specified"));
    if (!sc::is_uint32(data_len)) return su::alloc_copy(TRACEMSG(
            "Invalid 'data_len' parameter specified: [" + sc::to_string(data_len) + "]"));
    try {
        uint32_t data_len_u32 = static_cast<uint32_t> (data_len);
        request->impl().send_response(data, data_len_u32);
        return nullptr;
    } catch (const std::exception& e) {
        return su::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

char* wilton_Request_send_file(
        wilton_Request* request,
        const char* file_path,
        int file_path_len,
        void* finalizer_ctx,
        void (*finalizer_cb)(
        void* finalizer_ctx,
        int sent_successfully)) /* noexcept */ {
    if (nullptr == request) return su::alloc_copy(TRACEMSG("Null 'request' parameter specified"));
    if (nullptr == file_path) return su::alloc_copy(TRACEMSG("Null 'file_path' parameter specified"));
    if (!sc::is_uint16_positive(file_path_len)) return su::alloc_copy(TRACEMSG(
            "Invalid 'file_path_len' parameter specified: [" + sc::to_string(file_path_len) + "]"));
    if (nullptr == finalizer_cb) return su::alloc_copy(TRACEMSG("Null 'finalizer_cb' parameter specified"));
    try {
        uint16_t file_path_len_u16 = static_cast<uint16_t> (file_path_len);
        std::string file_path_str{file_path, file_path_len_u16};
        request->impl().send_file(std::move(file_path_str),
                [finalizer_ctx, finalizer_cb](bool success) {
                    int success_int = success ? 1 : 0;
                    finalizer_cb(finalizer_ctx, success_int);
                });
        return nullptr;
    } catch (const std::exception& e) {
        return su::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

char* wilton_Request_send_mustache(
        wilton_Request* request,
        const char* mustache_file_path,
        int mustache_file_path_len,
        const char* values_json,
        int values_json_len) /* noexcept */ {
    if (nullptr == request) return su::alloc_copy(TRACEMSG("Null 'request' parameter specified"));
    if (nullptr == mustache_file_path) return su::alloc_copy(TRACEMSG("Null 'mustache_file_path' parameter specified"));
    if (!sc::is_uint16_positive(mustache_file_path_len)) return su::alloc_copy(TRACEMSG(
            "Invalid 'mustache_file_path_len' parameter specified: [" + sc::to_string(mustache_file_path_len) + "]"));
    if (nullptr == values_json) return su::alloc_copy(TRACEMSG("Null 'values_json' parameter specified"));
    if (!sc::is_uint32_positive(values_json_len)) return su::alloc_copy(TRACEMSG(
            "Invalid 'values_json_len' parameter specified: [" + sc::to_string(values_json_len) + "]"));
    try {
        uint16_t mustache_file_path_len_u16 = static_cast<uint16_t> (mustache_file_path_len);
        std::string mustache_file_path_str{mustache_file_path, mustache_file_path_len_u16};
        uint32_t values_json_len_u32 = static_cast<uint32_t> (values_json_len);
        std::string values_json_str{values_json, values_json_len_u32};
        ss::json_value json = ss::load_json_from_string(values_json_str);
        request->impl().send_mustache(std::move(mustache_file_path_str), std::move(json));
        return nullptr;
    } catch (const std::exception& e) {
        return su::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

char* wilton_Request_send_later(
        wilton_Request* request,
        wilton_ResponseWriter** writer_out) /* noexcept */ {
    if (nullptr == request) return su::alloc_copy(TRACEMSG("Null 'request' parameter specified"));    
    if (nullptr == writer_out) return su::alloc_copy(TRACEMSG("Null 'writer_out' parameter specified"));
    try {
        ws::response_writer writer = request->impl().send_later();
        wilton_ResponseWriter* writer_ptr = new wilton_ResponseWriter(std::move(writer));
        *writer_out = writer_ptr;
        return nullptr;
    } catch (const std::exception& e) {
        return su::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

char* wilton_ResponseWriter_send(
        wilton_ResponseWriter* writer,
        const char* data,
        int data_len) /* noexcept */ {
    if (nullptr == writer) return su::alloc_copy(TRACEMSG("Null 'writer' parameter specified"));
    if (nullptr == data) return su::alloc_copy(TRACEMSG("Null 'data' parameter specified"));
    if (!sc::is_uint32(data_len)) return su::alloc_copy(TRACEMSG(
            "Invalid 'data_len' parameter specified: [" + sc::to_string(data_len) + "]"));
    try {
        uint32_t data_len_u32 = static_cast<uint32_t> (data_len);
        writer->impl().send(data, data_len_u32);
        delete writer;
        return nullptr;
    } catch (const std::exception& e) {
        delete writer;
        return su::alloc_copy(TRACEMSG("WRITER_INVALIDATED\n" + e.what() + "\nException raised"));
    }    
}

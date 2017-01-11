/* 
 * File:   wiltoncall_server.cpp
 * Author: alex
 *
 * Created on January 11, 2017, 10:05 AM
 */

#include "call/wiltoncall_internal.hpp"

#include "wilton/wilton.h"
#include "wilton/wiltoncall.h"

namespace wilton {
namespace server {

namespace { //anonymous

namespace ss = staticlib::serialization;

common::handle_registry<wilton_Server>& static_server_registry() {
    static common::handle_registry<wilton_Server> registry;
    return registry;
}

common::handle_registry<wilton_Request>& static_request_registry() {
    static common::handle_registry<wilton_Request> registry;
    return registry;
}

common::handle_registry<wilton_ResponseWriter>& static_response_writer_registry() {
    static common::handle_registry<wilton_ResponseWriter> registry;
    return registry;
}

void send_system_error(int64_t requestHandle, std::string errmsg) {
    wilton_Request* request = static_request_registry().remove(requestHandle);
    if (nullptr != request) {
        std::string conf{R"({"statusCode": 500, "statusMessage": "Server Error"})"};
        wilton_Request_set_response_metadata(request, conf.c_str(), conf.length());
        wilton_Request_send_response(request, errmsg.c_str(), errmsg.length());
        static_request_registry().put(request);
    }
}

} // namespace

std::string server_create(const std::string& data) {
    (void) data;
    return "";
//    if (nullptr == object) throw common::WiltonInternalException(TRACEMSG(
//            "Required parameter 'gateway' not specified"));
//    ss::JsonValue json = ss::load_json_from_string(data);
//    auto conf_in = ss::load_json_from_string(data);
//    auto views = extract_views(conf_in);
//    auto conf = ss::dump_json_to_string(conf_in);
//    ServerJniCtx ctx{object, views};
//    auto paths = create_paths(views, ctx);
//    auto paths_pass = wrap_paths(paths);
//    wilton_Server* server = nullptr;
//    char* err = wilton_Server_create(std::addressof(server),
//            conf.c_str(), conf.length(), paths_pass.data(), paths_pass.size());
//    if (nullptr != err) common::throw_wilton_error(err, TRACEMSG(err));
//    int64_t handle = static_server_registry().put(server, std::move(ctx));
//    return ss::dump_json_to_string({
//        { "serverHandle", handle}
//    });
}

std::string server_stop(const std::string& data) {
    // json parse
    ss::JsonValue json = ss::load_json_from_string(data);
    int64_t handle = -1;
    for (const ss::JsonField& fi : json.as_object()) {
        auto& name = fi.name();
        if ("serverHandle" == name) {
            handle = common::get_json_int64(fi);
        } else {
            throw common::WiltonInternalException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw common::WiltonInternalException(TRACEMSG(
            "Required parameter 'serverHandle' not specified"));
    // get handle
    wilton_Server* server = static_server_registry().remove(handle);
    if (nullptr == server) throw common::WiltonInternalException(TRACEMSG(
            "Invalid 'serverHandle' parameter specified"));
    // call wilton
    char* err = wilton_Server_stop(server);
    if (nullptr != err) {
        static_server_registry().put(server);
        common::throw_wilton_error(err, TRACEMSG(err));
    }
    return "{}";
}

std::string request_get_metadata(const std::string& data) {
    // json parse
    ss::JsonValue json = ss::load_json_from_string(data);
    int64_t handle = -1;
    for (const ss::JsonField& fi : json.as_object()) {
        auto& name = fi.name();
        if ("requestHandle" == name) {
            handle = common::get_json_int64(fi);
        } else {
            throw common::WiltonInternalException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw common::WiltonInternalException(TRACEMSG(
            "Required parameter 'requestHandle' not specified"));
    // get handle
    wilton_Request* request = static_request_registry().remove(handle);
    if (nullptr == request) throw common::WiltonInternalException(TRACEMSG(
            "Invalid 'requestHandle' parameter specified"));
    // call wilton
    char* out;
    int out_len;
    char* err = wilton_Request_get_request_metadata(request,
            std::addressof(out), std::addressof(out_len));
    static_request_registry().put(request);
    if (nullptr != err) {
        common::throw_wilton_error(err, TRACEMSG(err));
    }
    return common::wrap_wilton_output(out, out_len);
}

std::string request_get_data(const std::string& data) {
    // json parse
    ss::JsonValue json = ss::load_json_from_string(data);
    int64_t handle = -1;
    for (const ss::JsonField& fi : json.as_object()) {
        auto& name = fi.name();
        if ("requestHandle" == name) {
            handle = common::get_json_int64(fi);
        } else {
            throw common::WiltonInternalException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw common::WiltonInternalException(TRACEMSG(
            "Required parameter 'requestHandle' not specified"));
    // get handle
    wilton_Request* request = static_request_registry().remove(handle);
    if (nullptr == request) throw common::WiltonInternalException(TRACEMSG(
            "Invalid 'requestHandle' parameter specified"));
    // call wilton
    char* out;
    int out_len;
    char* err = wilton_Request_get_request_data(request,
            std::addressof(out), std::addressof(out_len));
    static_request_registry().put(request);
    if (nullptr != err) common::throw_wilton_error(err, TRACEMSG(err));
    return common::wrap_wilton_output(out, out_len);
}

std::string request_get_data_filename(const std::string& data) {
    // json parse
    ss::JsonValue json = ss::load_json_from_string(data);
    int64_t handle = -1;
    for (const ss::JsonField& fi : json.as_object()) {
        auto& name = fi.name();
        if ("requestHandle" == name) {
            handle = common::get_json_int64(fi);
        } else {
            throw common::WiltonInternalException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw common::WiltonInternalException(TRACEMSG(
            "Required parameter 'requestHandle' not specified"));
    // get handle
    wilton_Request* request = static_request_registry().remove(handle);
    if (nullptr == request) throw common::WiltonInternalException(TRACEMSG(
            "Invalid 'requestHandle' parameter specified"));
    // call wilton
    char* out;
    int out_len;
    char* err = wilton_Request_get_request_data_filename(request,
            std::addressof(out), std::addressof(out_len));
    static_request_registry().put(request);
    if (nullptr != err) common::throw_wilton_error(err, TRACEMSG(err));
    return common::wrap_wilton_output(out, out_len);
}

std::string request_set_response_metadata(const std::string& data) {
    // json parse
    ss::JsonValue json = ss::load_json_from_string(data);
    int64_t handle = -1;
    std::string metadata = common::empty_string();
    for (const ss::JsonField& fi : json.as_object()) {
        auto& name = fi.name();
        if ("requestHandle" == name) {
            handle = common::get_json_int64(fi);
        } else if ("metadata" == name) {
            metadata = ss::dump_json_to_string(fi.value());
        } else {
            throw common::WiltonInternalException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw common::WiltonInternalException(TRACEMSG(
            "Required parameter 'requestHandle' not specified"));
    if (metadata.empty()) throw common::WiltonInternalException(TRACEMSG(
            "Required parameter 'metadata' not specified"));
    // get handle
    wilton_Request* request = static_request_registry().remove(handle);
    if (nullptr == request) throw common::WiltonInternalException(TRACEMSG(
            "Invalid 'requestHandle' parameter specified"));
    // call wilton
    char* err = wilton_Request_set_response_metadata(request, metadata.c_str(), metadata.length());
    static_request_registry().put(request);
    if (nullptr != err) common::throw_wilton_error(err, TRACEMSG(err));
    return "{}";
}

std::string request_send_response(const std::string& data) {
    // json parse
    ss::JsonValue json = ss::load_json_from_string(data);
    int64_t handle = -1;
    auto rdata = std::ref(common::empty_string());
    for (const ss::JsonField& fi : json.as_object()) {
        auto& name = fi.name();
        if ("requestHandle" == name) {
            handle = common::get_json_int64(fi);
        } else if ("data" == name) {
            rdata = fi.as_string();
        } else {
            throw common::WiltonInternalException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw common::WiltonInternalException(TRACEMSG(
            "Required parameter 'requestHandle' not specified"));
    const std::string& request_data = rdata.get().empty() ? "{}" : rdata.get();
    // get handle
    wilton_Request* request = static_request_registry().remove(handle);
    if (nullptr == request) throw common::WiltonInternalException(TRACEMSG(
            "Invalid 'requestHandle' parameter specified"));
    // call wilton
    char* err = wilton_Request_send_response(request, request_data.c_str(), request_data.length());
    static_request_registry().put(request);
    if (nullptr != err) common::throw_wilton_error(err, TRACEMSG(err));
    return "{}";
}

std::string request_send_temp_file(const std::string& data) {
    // json parse
    ss::JsonValue json = ss::load_json_from_string(data);
    int64_t handle = -1;
    std::string file = common::empty_string();
    for (const ss::JsonField& fi : json.as_object()) {
        auto& name = fi.name();
        if ("requestHandle" == name) {
            handle = common::get_json_int64(fi);
        } else if ("filePath" == name) {
            file = common::get_json_string(fi);
        } else {
            throw common::WiltonInternalException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw common::WiltonInternalException(TRACEMSG(
            "Required parameter 'requestHandle' not specified"));
    if (file.empty()) throw common::WiltonInternalException(TRACEMSG(
            "Required parameter 'filePath' not specified"));
    // get handle
    wilton_Request* request = static_request_registry().remove(handle);
    if (nullptr == request) throw common::WiltonInternalException(TRACEMSG(
            "Invalid 'requestHandle' parameter specified"));
    // call wilton
    char* err = wilton_Request_send_file(request, file.c_str(), file.length(),
            new std::string(file.data(), file.length()),
            [](void* ctx, int) {
                std::string* filePath_passed = static_cast<std::string*> (ctx);
                std::remove(filePath_passed->c_str());
                        delete filePath_passed;
            });
    static_request_registry().put(request);
    if (nullptr != err) common::throw_wilton_error(err, TRACEMSG(err));
    return "{}";
}

std::string request_send_mustache(const std::string& data) {
    // json parse
    ss::JsonValue json = ss::load_json_from_string(data);
    int64_t handle = -1;
    auto rfile = std::ref(common::empty_string());
    std::string values = common::empty_string();
    for (const ss::JsonField& fi : json.as_object()) {
        auto& name = fi.name();
        if ("requestHandle" == name) {
            handle = common::get_json_int64(fi);
        } else if ("mustacheFilePath" == name) {
            rfile = common::get_json_string(fi);
        } else if ("values" == name) {
            values = ss::dump_json_to_string(fi.value());
        } else {
            throw common::WiltonInternalException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw common::WiltonInternalException(TRACEMSG(
            "Required parameter 'requestHandle' not specified"));
    if (rfile.get().empty()) throw common::WiltonInternalException(TRACEMSG(
            "Required parameter 'mustacheFilePath' not specified"));
    if (values.empty()) {
        values = "{}";
    }
    const std::string& file = rfile.get();
    // get handle
    wilton_Request* request = static_request_registry().remove(handle);
    if (nullptr == request) throw common::WiltonInternalException(TRACEMSG(
            "Invalid 'requestHandle' parameter specified"));
    // call wilton
    char* err = wilton_Request_send_mustache(request, file.c_str(), file.length(),
            values.c_str(), values.length());
    static_request_registry().put(request);
    if (nullptr != err) common::throw_wilton_error(err, TRACEMSG(err));
    return "{}";
}

std::string request_send_later(const std::string& data) {
    // json parse
    ss::JsonValue json = ss::load_json_from_string(data);
    int64_t handle = -1;
    for (const ss::JsonField& fi : json.as_object()) {
        auto& name = fi.name();
        if ("requestHandle" == name) {
            handle = common::get_json_int64(fi);
        } else {
            throw common::WiltonInternalException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw common::WiltonInternalException(TRACEMSG(
            "Required parameter 'requestHandle' not specified"));
    // get handle
    wilton_Request* request = static_request_registry().remove(handle);
    if (nullptr == request) throw common::WiltonInternalException(TRACEMSG(
            "Invalid 'requestHandle' parameter specified"));
    // call wilton
    wilton_ResponseWriter* writer;
    char* err = wilton_Request_send_later(request, std::addressof(writer));
    static_request_registry().put(request);
    if (nullptr != err) common::throw_wilton_error(err, TRACEMSG(err));
    int64_t rwhandle = static_response_writer_registry().put(writer);
    return ss::dump_json_to_string({
        { "responseWriterHandle", rwhandle}
    });
}

std::string request_send_with_response_writer(const std::string& data) {
    // json parse
    ss::JsonValue json = ss::load_json_from_string(data);
    int64_t handle = -1;
    auto rdata = std::ref(common::empty_string());
    for (const ss::JsonField& fi : json.as_object()) {
        auto& name = fi.name();
        if ("responseWriterHandle" == name) {
            handle = common::get_json_int64(fi);
        } else if ("data" == name) {
            rdata = fi.as_string();
        } else {
            throw common::WiltonInternalException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw common::WiltonInternalException(TRACEMSG(
            "Required parameter 'responseWriterHandle' not specified"));
    const std::string& request_data = rdata.get().empty() ? "{}" : rdata.get();
    // get handle, note: won't be put back - one-off operation   
    wilton_ResponseWriter* writer = static_response_writer_registry().remove(handle);
    if (nullptr == writer) throw common::WiltonInternalException(TRACEMSG(
            "Invalid 'responseWriterHandle' parameter specified"));
    // call wilton
    char* err = wilton_ResponseWriter_send(writer, request_data.c_str(), request_data.length());
    if (nullptr != err) common::throw_wilton_error(err, TRACEMSG(err));
    return "{}";
}

} // namespace
}

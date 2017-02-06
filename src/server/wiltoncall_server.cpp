/* 
 * File:   wiltoncall_server.cpp
 * Author: alex
 *
 * Created on January 11, 2017, 10:05 AM
 */

#include "call/wiltoncall_internal.hpp"

#include <list>

#include "staticlib/config.hpp"

#include "wilton/wilton.h"
#include "wilton/wiltoncall.h"

#include "logging/logging_internal.hpp"

namespace wilton {
namespace server {

namespace { //anonymous

namespace sc = staticlib::config;
namespace ss = staticlib::serialization;

class HttpView {
public:
    std::string method;
    std::string path;
    ss::JsonValue callbackScript;

    HttpView(const HttpView&) = delete;

    HttpView& operator=(const HttpView&) = delete;

    HttpView(HttpView&& other) :
    method(std::move(other.method)),
    path(std::move(other.path)),
    callbackScript(std::move(other.callbackScript)) { }

    HttpView& operator=(HttpView&&) = delete;

    HttpView(const ss::JsonValue& json) {
        if (ss::JsonType::OBJECT != json.type()) throw common::WiltonInternalException(TRACEMSG(
                "Invalid 'views' entry: must be an 'object'," +
                " entry: [" + ss::dump_json_to_string(json) + "]"));
        for (const ss::JsonField& fi : json.as_object()) {
            auto& name = fi.name();
            if ("method" == name) {
                method = common::get_json_string(fi);
            } else if ("path" == name) {
                path = common::get_json_string(fi);
            } else if ("callbackScript" == name) {
                common::check_json_callback_script(fi);
                callbackScript = fi.value().clone();
            } else {
                throw common::WiltonInternalException(TRACEMSG("Unknown data field: [" + name + "]"));
            }
        }
    }
};

class HttpPathDeleter {
public:
    void operator()(wilton_HttpPath* path) {
        wilton_HttpPath_destroy(path);
    }
};

class ServerCtx {
    // iterators must be permanent
    std::list<ss::JsonValue> callbackScripts;

public:
    ServerCtx(const ServerCtx&) = delete;

    ServerCtx& operator=(const ServerCtx&) = delete;

    ServerCtx(ServerCtx&& other) :
    callbackScripts(std::move(other.callbackScripts)) { }

    ServerCtx& operator=(ServerCtx&&) = delete;

    ServerCtx() { }

    ss::JsonValue& add_callback(const ss::JsonValue& callback) {
        callbackScripts.emplace_back(callback.clone());
        return callbackScripts.back();
    }
};

common::payload_handle_registry<wilton_Server, ServerCtx>& static_server_registry() {
    static common::payload_handle_registry<wilton_Server, ServerCtx> registry;
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

std::vector<HttpView> extract_and_delete_views(ss::JsonValue& conf) {
    std::vector<ss::JsonField>& fields = conf.as_object_or_throw(TRACEMSG(
            "Invalid configuration object specified: invalid type," +
            " conf: [" + ss::dump_json_to_string(conf) + "]"));
    std::vector<HttpView> views;
    uint32_t i = 0;
    for (auto it = fields.begin(); it != fields.end(); ++it) {
        ss::JsonField& fi = *it;
        if ("views" == fi.name()) {
            if (ss::JsonType::ARRAY != fi.type()) throw common::WiltonInternalException(TRACEMSG(
                    "Invalid configuration object specified: 'views' attr is not a list," +
                    " conf: [" + ss::dump_json_to_string(conf) + "]"));
            if (0 == fi.as_array().size()) throw common::WiltonInternalException(TRACEMSG(
                    "Invalid configuration object specified: 'views' attr is am empty list," +
                    " conf: [" + ss::dump_json_to_string(conf) + "]"));
            for (auto& va : fi.as_array()) {
                if (ss::JsonType::OBJECT != va.type()) throw common::WiltonInternalException(TRACEMSG(
                        "Invalid configuration object specified: 'views' is not a 'object'," +
                        "index: [" + sc::to_string(i) + "], conf: [" + ss::dump_json_to_string(conf) + "]"));
                views.emplace_back(HttpView(va));
            }
            // drop views attr and return immediately (iters are invalidated)
            fields.erase(it);
            return views;
        }
        i++;
    }
    throw common::WiltonInternalException(TRACEMSG(
            "Invalid configuration object specified: 'views' list not specified," +
            " conf: [" + ss::dump_json_to_string(conf) + "]"));
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

std::vector<std::unique_ptr<wilton_HttpPath, HttpPathDeleter>> create_paths(
        const std::vector<HttpView>& views, ServerCtx& ctx) {
    // assert(views.size() == ctx.get_modules_names().size())
    std::vector<std::unique_ptr<wilton_HttpPath, HttpPathDeleter>> res;
    for (auto& vi : views) {
        ss::JsonValue& cbs_to_pass = ctx.add_callback(vi.callbackScript);
        wilton_HttpPath* ptr = nullptr;
        auto err = wilton_HttpPath_create(std::addressof(ptr), vi.method.c_str(), vi.method.length(),
                vi.path.c_str(), vi.path.length(), static_cast<void*> (std::addressof(cbs_to_pass)),
                [](void* passed, wilton_Request* request) {
                    int64_t requestHandle = static_request_registry().put(request);
                    ss::JsonValue* cb_ptr = static_cast<ss::JsonValue*> (passed);
                    ss::JsonValue params = cb_ptr->clone();
                    // params structure is pre-checked
                    std::vector<ss::JsonValue>& args = params.getattr_or_throw("args").as_array_or_throw();
                    args.emplace_back(requestHandle);
                    std::string params_str = ss::dump_json_to_string(params);
                    // output will be ignored
                    char* out;
                    int out_len;
                    auto err = wiltoncall_runscript(params_str.c_str(), static_cast<int> (params_str.length()),
                            std::addressof(out), std::addressof(out_len));
                    if (nullptr == err) {
                        wilton_free(out);
                    } else {
                        std::string msg = TRACEMSG(err);
                        wilton_free(err);
                        log_error("wilton.server", msg);
                        send_system_error(requestHandle, msg);
                    }
                    static_request_registry().remove(requestHandle);
                });
        if (nullptr != err) throw common::WiltonInternalException(TRACEMSG(err));
        res.emplace_back(ptr, HttpPathDeleter());
    }
    return res;
}

std::vector<wilton_HttpPath*> wrap_paths(std::vector<std::unique_ptr<wilton_HttpPath, HttpPathDeleter>>&paths) {
    std::vector<wilton_HttpPath*> res;
    for (auto& pa : paths) {
        res.push_back(pa.get());
    }
    return res;
}

} // namespace

std::string server_create(const std::string& data) {
    ss::JsonValue json = ss::load_json_from_string(data);
    auto conf_in = ss::load_json_from_string(data);
    auto views = extract_and_delete_views(conf_in);
    auto conf = ss::dump_json_to_string(conf_in);
    ServerCtx ctx;
    auto paths = create_paths(views, ctx);
    auto paths_pass = wrap_paths(paths);
    wilton_Server* server = nullptr;
    char* err = wilton_Server_create(std::addressof(server),
            conf.c_str(), conf.length(), paths_pass.data(), paths_pass.size());
    if (nullptr != err) common::throw_wilton_error(err, TRACEMSG(err));
    int64_t handle = static_server_registry().put(server, std::move(ctx));
    return ss::dump_json_to_string({
        { "serverHandle", handle}
    });
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
    auto pa = static_server_registry().remove(handle);
    if (nullptr == pa.first) throw common::WiltonInternalException(TRACEMSG(
            "Invalid 'serverHandle' parameter specified"));
    // call wilton
    char* err = wilton_Server_stop(pa.first);
    if (nullptr != err) {
        static_server_registry().put(pa.first, std::move(pa.second));
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

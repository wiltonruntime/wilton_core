/* 
 * File:   duktape_engine.cpp
 * Author: alex
 * 
 * Created on May 20, 2017, 2:09 PM
 */

#include "duktape/duktape_engine.hpp"

#include <cstring>
#include <functional>
#include <memory>

#include "duktape.h"

#include "staticlib/io.hpp"
#include "staticlib/json.hpp"
#include "staticlib/support.hpp"
#include "staticlib/tinydir.hpp"
#include "staticlib/pimpl/forward_macros.hpp"

#include "wilton/wiltoncall.h"

#include "common/wilton_internal_exception.hpp"

namespace wilton {
namespace duktape {

namespace { // anonymous

void fatal_handler(duk_context*, duk_errcode_t code, const char* msg) {
    throw common::wilton_internal_exception(TRACEMSG("Duktape fatal error,"
            " code: [" + sl::support::to_string(code) + "], message: [" + msg + "]"));
}

void ctx_deleter(duk_context* ctx) {
    if (nullptr != ctx) {
        duk_destroy_heap(ctx);
    }
}

void pop_stack(duk_context* ctx) {
    duk_pop_n(ctx, duk_get_top(ctx));
}

std::string format_error(duk_context* ctx) {
    if (duk_is_error(ctx, -1)) {
        /* Accessing .stack might cause an error to be thrown, so wrap this
         * access in a duk_safe_call() if it matters.
         */
        duk_get_prop_string(ctx, -1, "stack");
        auto res = std::string(duk_safe_to_string(ctx, -1));
        duk_pop(ctx);
        return res;
    } else {
        /* Non-Error value, coerce safely to string. */
        return std::string(duk_safe_to_string(ctx, -1));
    }
}

std::string read_file(const std::string& path) {
    auto src = sl::tinydir::file_source(path);
    auto sink = sl::io::string_sink();
    sl::io::copy_all(src, sink);
    return sink.get_string();
}

duk_ret_t load_func(duk_context* ctx) {
    std::string path = "";
    try {        
        size_t path_len;
        const char* path_ptr = duk_get_lstring(ctx, 0, std::addressof(path_len));
        if (nullptr == path_ptr) {
            throw common::wilton_internal_exception(TRACEMSG("Invalid 'load' arguments"));
        }    
        path = std::string(path_ptr, path_len);
        // read file
        auto code = read_file(path);
        // compile source
        duk_push_lstring(ctx, code.c_str(), code.length());
        duk_push_lstring(ctx, path.c_str(), path.length());
        auto err = duk_pcompile(ctx, DUK_COMPILE_EVAL);
        if (DUK_EXEC_SUCCESS == err) {
            err = duk_pcall(ctx, 0);
        }
        if (DUK_EXEC_SUCCESS != err) {
            std::string msg = format_error(ctx);
            duk_pop(ctx);
            throw common::wilton_internal_exception(TRACEMSG(msg + "\nCall error"));
        } else {
            duk_pop(ctx);
            duk_push_true(ctx);
        }
        return 1;
    } catch (const std::exception& e) {
        throw common::wilton_internal_exception(TRACEMSG(e.what() + 
                "\nError loading script, path: [" + path + "]").c_str());
    } catch (...) {
        throw common::wilton_internal_exception(TRACEMSG(
                "Error loading script, path: [" + path + "]").c_str());
    }    
}

duk_ret_t wiltoncall_func(duk_context* ctx) {
    size_t name_len;
    const char* name = duk_get_lstring(ctx, 0, std::addressof(name_len));
    if (nullptr == name) {
        name = "";
        name_len = 0;
    }
    size_t input_len;
    const char* input = duk_get_lstring(ctx, 1, std::addressof(input_len));
    if (nullptr == input) {
        input = "";
        input_len = 0;
    }
    char* out;
    int out_len;
    auto err = wiltoncall(name, static_cast<int> (name_len), input, static_cast<int> (input_len),
            std::addressof(out), std::addressof(out_len));
    if (nullptr == err) {
        duk_push_lstring(ctx, out, out_len);
        wilton_free(out);
        return 1;
    } else {
        auto msg = TRACEMSG(err + "\n'wiltoncall' error for name: [" + name + "]");
        wilton_free(err);
        throw common::wilton_internal_exception(msg);
    }
}

void register_c_func(duk_context* ctx, const std::string& name, duk_c_function fun, size_t argnum) {
    duk_push_global_object(ctx);
    duk_push_c_function(ctx, fun, argnum);
    duk_put_prop_string(ctx, -2, name.c_str());
    duk_pop(ctx);
}

void check__scripts_dir(const std::string& path_to_scripts_dir) {
    bool has_requirejs = false;
    bool has_modules = false;
    auto vec = sl::tinydir::list_directory(path_to_scripts_dir);
    for (auto& el : vec) {
        if ("requirejs" == el.filename()) {
           has_requirejs = true; 
        }
        if ("modules" == el.filename()) {
            has_modules = true;
        }
    }
    if (!(has_requirejs && has_modules)) throw common::wilton_internal_exception(TRACEMSG(
            "Invalid scripts directory specified," +
            " it must contain 'requirejs' and 'modules' directories,"
            " path: [" + path_to_scripts_dir + "]"));
}

void eval_js(duk_context* ctx, const std::string& code) {
    auto err = duk_peval_lstring(ctx, code.c_str(), code.length());
    if (DUK_EXEC_SUCCESS != err) {
        throw common::wilton_internal_exception(TRACEMSG(format_error(ctx) +
                "\nDuktape engine eval error"));
    }
}

} // namespace

class duktape_engine::impl : public sl::pimpl::object::impl {
    std::unique_ptr<duk_context, std::function<void(duk_context*)>> dukctx;
    
public:
    impl(const std::string& path_to_scripts_dir) :
    dukctx(duk_create_heap(nullptr, nullptr, nullptr, nullptr, fatal_handler), ctx_deleter) {
        auto ctx = dukctx.get();
        if (nullptr == ctx) throw common::wilton_internal_exception(TRACEMSG(
                "Error creating Duktape context"));
        auto def = sl::support::defer([ctx]() STATICLIB_NOEXCEPT {
            pop_stack(ctx);
        });
        register_c_func(ctx, "WILTON_load", load_func, 1);
        register_c_func(ctx, "WILTON_wiltoncall", wiltoncall_func, 2);        
        check__scripts_dir(path_to_scripts_dir);
        eval_js(ctx, "WILTON_REQUIREJS_DIRECTORY = \"" + path_to_scripts_dir + "/requirejs/\"");
        auto rcf = sl::json::dumps({
            { "waitSeconds", 30 },
            { "baseUrl", path_to_scripts_dir + "/modules/" }
        });
        std::replace(rcf.begin(), rcf.end(), '\n', ' ');
        eval_js(ctx, "WILTON_REQUIREJS_CONFIG = '" + rcf + "'");
        auto code = read_file(path_to_scripts_dir + "/requirejs/wilton-duktape.js");
        eval_js(ctx, code);
    }

    std::string run_script(duktape_engine&, const std::string& callback_script_json) {
        auto ctx = dukctx.get();
        auto def = sl::support::defer([ctx]() STATICLIB_NOEXCEPT {
            pop_stack(ctx);
        });
        duk_get_global_string(ctx, "WILTON_run");
        duk_push_string(ctx, callback_script_json.c_str());
        auto err = duk_pcall(ctx, 1);
        if (DUK_EXEC_SUCCESS != err) {
            throw common::wilton_internal_exception(TRACEMSG(format_error(ctx) + 
                    "\nError running script: [" + callback_script_json + "]"));
        }
        if (DUK_TYPE_STRING == duk_get_type(ctx, -1)) {
            size_t len;
            const char* str = duk_get_lstring(ctx, -1, std::addressof(len));
            if (len > 0) {
                return std::string(str, len);
            }
        }
        return "";
    }    
};

PIMPL_FORWARD_CONSTRUCTOR(duktape_engine, (const std::string&), (), common::wilton_internal_exception)
PIMPL_FORWARD_METHOD(duktape_engine, std::string, run_script, (const std::string&), (), common::wilton_internal_exception)


} // namespace
}

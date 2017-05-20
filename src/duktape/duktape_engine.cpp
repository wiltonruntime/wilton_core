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
#include <vector>
#include <unordered_map>

#include "duktape.h"

#include "staticlib/support.hpp"
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
    } else {
        duk_push_string(ctx, err);
        wilton_free(err);
    }
    return 1;
}

class duktape_error_checker {
public:
    void operator=(duk_int_t err) {
        if (DUK_EXEC_SUCCESS != err) {
            throw common::wilton_internal_exception(TRACEMSG(
                    "Duktape error, code: [" + sl::support::to_string(err) + "]"));
        }
    }
};

} // namespace

class duktape_engine::impl : public sl::pimpl::object::impl {
    std::unique_ptr<duk_context, std::function<void(duk_context*)>> ctx;
    std::unordered_map<std::string, std::vector<unsigned char>> cache;
    
public:

    impl() :
    ctx(duk_create_heap(nullptr, nullptr, nullptr, nullptr, fatal_handler), ctx_deleter) {
        if (nullptr == ctx.get()) throw common::wilton_internal_exception(TRACEMSG(
                "Error creating Duktape context"));
        duk_push_global_object(ctx.get());
        duk_push_c_function(ctx.get(), wiltoncall_func, 2);
        duk_put_prop_string(ctx.get(), -2, "wiltoncall");
        duk_pop(ctx.get());
    }

    std::string run_script(duktape_engine&, const std::string& script_body, const std::string& filename) {
        duktape_error_checker ec;
        auto it = cache.find(filename);
        if (it == cache.end()) {
            auto def = sl::support::defer([this]() STATICLIB_NOEXCEPT {
                this->pop_stack();
            });
            duk_push_string(ctx.get(), script_body.c_str());
            duk_push_string(ctx.get(), filename.c_str());
            ec = duk_pcompile(ctx.get(), DUK_COMPILE_EVAL);
            duk_dump_function(ctx.get());
            size_t len;
            auto buf_ptr = duk_get_buffer(ctx.get(), -1, std::addressof(len));
            if (nullptr == buf_ptr) {
                throw common::wilton_internal_exception(TRACEMSG(
                        "Error obtaining Duktape bytecode buffer"));
            }
            std::vector<unsigned char> vec;
            vec.resize(len);
            std::memcpy(vec.data(), buf_ptr, len);
            auto pa = cache.emplace(filename, std::move(vec));
            it = pa.first;
        }
        auto def = sl::support::defer([this]() STATICLIB_NOEXCEPT {
            this->pop_stack();
        });
        std::vector<unsigned char>& bytecode = it->second;
        duk_push_fixed_buffer(ctx.get(), bytecode.size());
        size_t load_size;
        auto load_ptr = duk_get_buffer(ctx.get(), -1, std::addressof(load_size));
        if (nullptr == load_ptr) {
            throw common::wilton_internal_exception(TRACEMSG(
                    "Error loading Duktape bytecode"));
        }
        std::memcpy(load_ptr, bytecode.data(), bytecode.size());
        duk_load_function(ctx.get());
        ec = duk_pcall(ctx.get(), 0);
        if (DUK_TYPE_STRING == duk_get_type(ctx.get(), -1)) {
            size_t len;
            const char* str = duk_get_lstring(ctx.get(), -1, std::addressof(len));
            if (len > 0) {
                return std::string(str, len);
            }
        }
        return "";
    }

private:
    void pop_stack() {
        duk_pop_n(ctx.get(), duk_get_top(ctx.get()));
    }
    
};

PIMPL_FORWARD_CONSTRUCTOR(duktape_engine, (), (), common::wilton_internal_exception)
PIMPL_FORWARD_METHOD(duktape_engine, std::string, run_script, (const std::string&)(const std::string&), (), common::wilton_internal_exception)


} // namespace
}

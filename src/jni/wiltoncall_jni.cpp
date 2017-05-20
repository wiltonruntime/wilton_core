/* 
 * File:   wiltoncall_jni.cpp
 * Author: alex
 *
 * Created on January 10, 2017, 6:34 PM
 */

#include "call/wiltoncall_internal.hpp"

#include <atomic>
#include <memory>
#include <string>

#include "jni.h"

#include "staticlib/config.hpp"
#include "staticlib/io.hpp"
#include "staticlib/utils.hpp"

#include "wilton/wilton.h"

#include "common/wilton_internal_exception.hpp"
#include "common/utils.hpp"
#include "jni/jni_config.hpp"
#include "jni_utils.hpp"

namespace { // anonymous

namespace sc = staticlib::config;
namespace si = staticlib::io;
namespace su = staticlib::utils;
namespace wc = wilton::common;
namespace wj = wilton::jni;


std::atomic<bool>& static_jvm_active() {
    static std::atomic<bool> flag{false};
    return flag;
}

// forward declaration
JNIEnv* get_jni_env();

class global_ref_deleter {
public:
    void operator()(jobject ref) {
        if (static_jvm_active().load()) {
            get_jni_env()->DeleteGlobalRef(ref);
        }
    }
};

class jni_ctx {
public:
    JavaVM* vm;
    std::unique_ptr<_jclass, global_ref_deleter> wiltonJniClass;
    jmethodID describeThrowableMethod;
    std::unique_ptr<_jclass, global_ref_deleter> wiltonGatewayInterface;
    jmethodID runScriptMethod;
    std::unique_ptr<_jclass, global_ref_deleter> wiltonExceptionClass;
    std::unique_ptr<_jobject, global_ref_deleter> wiltonGatewayObject;

    jni_ctx(const jni_ctx&) = delete;

    jni_ctx& operator=(const jni_ctx&) = delete;

    jni_ctx(jni_ctx&&) = delete;

    jni_ctx& operator=(jni_ctx&& other) {
        this->vm = other.vm;
        other.vm = nullptr;
        this->wiltonJniClass = std::move(other.wiltonJniClass);
        this->describeThrowableMethod = other.describeThrowableMethod;
        other.describeThrowableMethod = nullptr;
        this->wiltonGatewayInterface = std::move(other.wiltonGatewayInterface);
        this->runScriptMethod = other.runScriptMethod;
        other.runScriptMethod = nullptr;
        this->wiltonExceptionClass = std::move(other.wiltonExceptionClass);
        this->wiltonGatewayObject = std::move(other.wiltonGatewayObject);
        return *this;
    }

    jni_ctx() { }

    jni_ctx(JavaVM* vm) :
    vm(vm) {
        // env
        JNIEnv* env;
        auto err = vm->GetEnv(reinterpret_cast<void**> (std::addressof(env)), WILTON_JNI_VERSION);
        if (JNI_OK != err) {
            throw wc::wilton_internal_exception(TRACEMSG("Cannot obtain JNI environment"));
        }
        // jni class
        this->wiltonJniClass = std::unique_ptr<_jclass, global_ref_deleter>(
                wj::find_java_class(env, WILTON_JNI_CLASS_SIGNATURE_STR), global_ref_deleter());
        // describe
        this->describeThrowableMethod = wj::find_java_method_static(env, this->wiltonJniClass.get(),
                WILTON_JNI_CLASS_DESCRIBETHROWABLE_METHOD_STR, WILTON_JNI_CLASS_DESCRIBETHROWABLE_METHOD_SIGNATURE_STR);
        // gateway
        this->wiltonGatewayInterface = std::unique_ptr<_jclass, global_ref_deleter>(
                wj::find_java_class(env, WILTON_JNI_GATEWAY_INTERFACE_SIGNATURE_STR), global_ref_deleter());
        // runscript
        this->runScriptMethod = wj::find_java_method(env, this->wiltonGatewayInterface.get(),
                WILTON_JNI_GATEWAY_RUNSCRIPT_METHOD_STR, WILTON_JNI_GATEWAY_RUNSCRIPT_METHOD_SIGNATURE_STR);
        // exception
        this->wiltonExceptionClass = std::unique_ptr<_jclass, global_ref_deleter>(
                wj::find_java_class(env, WILTON_JNI_EXCEPTION_CLASS_SIGNATURE_STR), global_ref_deleter());
    }
    
    void set_gateway_object(JNIEnv* env, jobject gateway) {
        auto ptr = std::unique_ptr<_jobject, global_ref_deleter>(
                static_cast<jobject> (env->NewGlobalRef(gateway)), global_ref_deleter());
        if (nullptr == ptr.get()) {
            throw wc::wilton_internal_exception(TRACEMSG("Cannot create global ref for specified gateway object"));
        }
        this->wiltonGatewayObject = std::move(ptr);
    }
};

jni_ctx& static_jni_ctx() {
    // will be destructed in JNI_OnUnload
    static jni_ctx* ctx = new jni_ctx();
    return *ctx;
}

JNIEnv* get_jni_env() {
    JavaVM* vm = static_jni_ctx().vm;
    JNIEnv* env;
    auto getenv_err = vm->GetEnv(reinterpret_cast<void**> (std::addressof(env)), WILTON_JNI_VERSION);
    switch (getenv_err) {
    case JNI_OK:
        return env;
    case JNI_EDETACHED:
        if (JNI_OK == vm->AttachCurrentThread(reinterpret_cast<void**>(std::addressof(env)), nullptr)) {
            return env;
        }
        // fall-through to report error to client
    default:
        throw wc::wilton_internal_exception(TRACEMSG("System error: cannot obtain JNI environment"));
    }
}

std::string describe_java_exception(JNIEnv* env, jthrowable exc) {
    jobject umsg = env->CallStaticObjectMethod(static_jni_ctx().wiltonJniClass.get(),
            static_jni_ctx().describeThrowableMethod, exc);
    if (!env->ExceptionCheck()) {
        std::string res = wj::jstring_to_str(env, static_cast<jstring>(umsg));
        env->DeleteLocalRef(umsg);
        return res;
    } else {
        env->ExceptionClear();
        return "EXC_DESCRIBE_ERROR";
    }
}

} // namespace

extern "C" {

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void*) {
    try {
        // move-assign static ctx
        static_jni_ctx() = jni_ctx(vm);
        // set init flag
        static_jvm_active().store(true);
        return WILTON_JNI_VERSION;
    } catch (const std::exception& e) {
        wc::dump_error(WILTON_STARTUP_ERR_DIR_STR, TRACEMSG(e.what() + "\nInitialization error"));
        return -1;
    }
}

// generally won't be called on most JVMs
// as ctx holds a bunch of global refs
JNIEXPORT void JNI_OnUnload(JavaVM*, void*) {
    delete std::addressof(static_jni_ctx());
    // flip init flag
    static_jvm_active().store(false);
}

JNIEXPORT void JNICALL WILTON_JNI_FUNCTION(wiltoninit)
(JNIEnv* env, jclass, jobject gateway, jstring logging_conf) {
    // check called once
    bool the_false = false;
    static std::atomic<bool> initilized{false};
    if (!initilized.compare_exchange_strong(the_false, true)) {
        env->ThrowNew(static_jni_ctx().wiltonExceptionClass.get(),
                TRACEMSG("'wiltoninit' was already called").c_str());
        return;
    }
    
    std::string lconf = "";
    try {
        // set gateway
        static_jni_ctx().set_gateway_object(env, gateway);
        // wiltoncalls
        std::string engine = "jni";
        auto err_init = wiltoncall_init(engine.c_str(), engine.length());
        if (nullptr != err_init) {
            wc::throw_wilton_error(err_init, TRACEMSG(err_init));
        }
        // init_logging
        lconf = wj::jstring_to_str(env, logging_conf);
        auto err_logging = wilton_logger_initialize(lconf.c_str(), static_cast<int>(lconf.length()));
        if (nullptr != err_logging) {
            wc::throw_wilton_error(err_logging, TRACEMSG(err_logging));
        }
    } catch (const std::exception& e) {
        env->ThrowNew(static_jni_ctx().wiltonExceptionClass.get(),
                TRACEMSG(e.what() + "\nWilton initialization error," +
                " logging_conf: [" + lconf + "]").c_str());
    }
}

JNIEXPORT jstring JNICALL WILTON_JNI_FUNCTION(wiltoncall)
(JNIEnv* env, jclass, jstring name, jstring data) {
    if (nullptr == name) {
        env->ThrowNew(static_jni_ctx().wiltonExceptionClass.get(),
                TRACEMSG("Null 'name' parameter specified").c_str());
        return nullptr;
    }
    if (nullptr == data) {
        env->ThrowNew(static_jni_ctx().wiltonExceptionClass.get(),
                TRACEMSG("Null 'data' parameter specified").c_str());
        return nullptr;
    }
    std::string name_string = wj::jstring_to_str(env, name);
    std::string data_string = wj::jstring_to_str(env, data);
    char* out;
    int out_len;
    auto err = wiltoncall(name_string.c_str(), static_cast<int>(name_string.length()), 
            data_string.c_str(), static_cast<int>(data_string.length()),
            std::addressof(out), std::addressof(out_len));
    if (nullptr == err) {
        jstring res = env->NewStringUTF(out);
        wilton_free(out);
        return res;
    } else {
        env->ThrowNew(static_jni_ctx().wiltonExceptionClass.get(), TRACEMSG(err + 
                "\n'wiltoncall' error for name: [" + name_string + "]").c_str());
        wilton_free(err);
        return nullptr;
    }
}

} // C

char* wiltoncall_runscript_jni(const char* json_in, int json_in_len, char** json_out, 
        int* json_out_len) /* noexcept */ {
    if (nullptr == json_in) return sl::utils::alloc_copy(TRACEMSG("Null 'json_in' parameter specified"));
    if (!sl::support::is_uint32(json_in_len)) return sl::utils::alloc_copy(TRACEMSG(
            "Invalid 'json_in_len' parameter specified: [" + sl::support::to_string(json_in_len) + "]"));
    if (nullptr == json_out) return sl::utils::alloc_copy(TRACEMSG("Null 'json_out' parameter specified"));
    if (nullptr == json_out_len) return sl::utils::alloc_copy(TRACEMSG("Null 'json_out_len' parameter specified"));
    try {
        jni_ctx& ctx = static_jni_ctx();
        JNIEnv* env = get_jni_env();
        jstring json_ustr = env->NewStringUTF(json_in);
        jobject res = env->CallObjectMethod(ctx.wiltonGatewayObject.get(),
                ctx.runScriptMethod, json_ustr);
        env->DeleteLocalRef(json_ustr);
        jthrowable exc = env->ExceptionOccurred();
        if (nullptr == exc) {
            std::string res_str = nullptr != res ?
            wj::jstring_to_str(env, static_cast<jstring> (res)) : "";
            *json_out = sl::utils::alloc_copy(res_str.c_str());
            *json_out_len = static_cast<int>(res_str.length());
            return nullptr;
        } else {
            env->ExceptionClear();
            std::string trace = describe_java_exception(env, exc);
            std::string msg = TRACEMSG(trace);
            return sl::utils::alloc_copy(msg.c_str());
        }
    } catch (const std::exception& e) {
        return sl::utils::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

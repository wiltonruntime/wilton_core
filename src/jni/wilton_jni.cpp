/* 
 * File:   wilton_jni.cpp
 * Author: alex
 *
 * Created on January 10, 2017, 6:34 PM
 */

#include <atomic>
#include <memory>
#include <string>

#include "jni.h"

#include "staticlib/config.hpp"
#include "staticlib/io.hpp"
#include "staticlib/utils.hpp"

#include "wilton/wilton.h"
#include "wilton/wiltoncall.h"

#include "common/WiltonInternalException.hpp"
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

class GlobalRefDeleter {
public:
    void operator()(jobject ref) {
        if (static_jvm_active().load()) {
            get_jni_env()->DeleteGlobalRef(ref);
        }
    }
};

class JniCtx {
public:
    JavaVM* vm;
    std::unique_ptr<_jclass, GlobalRefDeleter> wiltonJniClass;
    jmethodID describeThrowableMethod;
    std::unique_ptr<_jclass, GlobalRefDeleter> wiltonGatewayInterface;
    jmethodID runScriptMethod;
    std::unique_ptr<_jclass, GlobalRefDeleter> wiltonExceptionClass;
    std::unique_ptr<_jobject, GlobalRefDeleter> wiltonGatewayObject;

    JniCtx(const JniCtx&) = delete;

    JniCtx& operator=(const JniCtx&) = delete;

    JniCtx(JniCtx&&) = delete;

    JniCtx& operator=(JniCtx&& other) {
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

    JniCtx() { }

    JniCtx(JavaVM* vm) :
    vm(vm) {
        // env
        JNIEnv* env;
        auto err = vm->GetEnv(reinterpret_cast<void**> (std::addressof(env)), WILTON_JNI_VERSION);
        if (JNI_OK != err) {
            throw wc::WiltonInternalException(TRACEMSG("Cannot obtain JNI environment"));
        }
        // jni class
        this->wiltonJniClass = std::unique_ptr<_jclass, GlobalRefDeleter>(
                wj::find_java_class(env, WILTON_JNI_CLASS_SIGNATURE_STR), GlobalRefDeleter());
        // describe
        this->describeThrowableMethod = wj::find_java_method(env, this->wiltonJniClass.get(),
                WILTON_JNI_CLASS_DESCRIBETHROWABLE_METHOD_STR, WILTON_JNI_CLASS_DESCRIBETHROWABLE_METHOD_SIGNATURE_STR);
        // gateway
        this->wiltonGatewayInterface = std::unique_ptr<_jclass, GlobalRefDeleter>(
                wj::find_java_class(env, WILTON_JNI_GATEWAY_INTERFACE_SIGNATURE_STR), GlobalRefDeleter());
        // runscript
        this->runScriptMethod = wj::find_java_method(env, this->wiltonGatewayInterface.get(),
                WILTON_JNI_GATEWAY_RUNSCRIPT_METHOD_STR, WILTON_JNI_GATEWAY_RUNSCRIPT_METHOD_SIGNATURE_STR);
        // exception
        this->wiltonExceptionClass = std::unique_ptr<_jclass, GlobalRefDeleter>(
                wj::find_java_class(env, WILTON_JNI_EXCEPTION_CLASS_SIGNATURE_STR), GlobalRefDeleter());
    }
    
    void set_gateway_object(JNIEnv* env, jobject gateway) {
        auto ptr = std::unique_ptr<_jobject, GlobalRefDeleter>(
                static_cast<jobject> (env->NewGlobalRef(gateway)), GlobalRefDeleter());
        if (nullptr == ptr.get()) {
            throw wc::WiltonInternalException(TRACEMSG("Cannot create global ref for specified gateway object"));
        }
        this->wiltonGatewayObject = std::move(ptr);
    }
};

JniCtx& static_jni_ctx() {
    // will be destructed in JNI_OnUnload
    static JniCtx* ctx = new JniCtx();
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
        if (JNI_OK == vm->AttachCurrentThread(std::addressof(env), nullptr)) {
            return env;
        }
        // fall-through to report error to client
    default:
        throw wc::WiltonInternalException(TRACEMSG("System error: cannot obtain JNI environment"));
    }
}

std::string describe_java_exception(JNIEnv* env, jthrowable exc) {
    jobject umsg = env->CallStaticObjectMethod(static_jni_ctx().wiltonJniClass.get(),
            static_jni_ctx().describeThrowableMethod, exc);
    if (!env->ExceptionOccurred()) {
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
        static_jni_ctx() = JniCtx(vm);
        // set init flag
        static_jvm_active().store(true);
        return WILTON_JNI_VERSION;
    } catch (const std::exception& e) {
        wc::dump_error(WILTON_STARTUP_ERR_DIR_STR, TRACEMSG(e.what() + "\nInitialization error"));
        return -1;
    }
}

JNIEXPORT void JNI_OnUnload(JavaVM*, void*) {
    delete std::addressof(static_jni_ctx());
    // flip init flag
    static_jvm_active().store(false);
}

JNIEXPORT void JNICALL WILTON_JNI_FUNCTION(wiltoninit)
(JNIEnv* env, jclass, jobject gateway, jstring logging_conf) {
    try {
    // set gateway
        static_jni_ctx().set_gateway_object(env, gateway);
        // wiltoncalls
        wiltoncall_init();
        // init_logging
        std::string lconf = wj::jstring_to_str(env, logging_conf);
        auto err = wilton_logger_initialize(lconf.c_str(), static_cast<int>(lconf.length()));
        if (nullptr != err) {
            wc::throw_wilton_error(err, TRACEMSG(err));
        }
    } catch (const std::exception& e) {
        env->ThrowNew(static_jni_ctx().wiltonExceptionClass.get(),
                TRACEMSG(e.what() + "\nWilton initialization error").c_str());
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
        return env->NewStringUTF(out);
    } else {
        env->ThrowNew(static_jni_ctx().wiltonExceptionClass.get(), TRACEMSG(err + 
                "\n'wiltoncall' error for name: [" + name_string + "]," +
                " data: [" + data_string + "]").c_str());
        wilton_free(err);
        return nullptr;
    }
}

} // C

char* wiltoncall_runscript(const char* json_in, int json_in_len, char** json_out, 
        int* json_out_len) /* noexcept */ {
    if (nullptr == json_in) return su::alloc_copy(TRACEMSG("Null 'json_in' parameter specified"));
    if (!su::is_uint32(json_in_len)) return su::alloc_copy(TRACEMSG(
            "Invalid 'json_in_len' parameter specified: [" + sc::to_string(json_in_len) + "]"));
    if (nullptr == json_out) return su::alloc_copy(TRACEMSG("Null 'json_out' parameter specified"));
    if (nullptr == json_out_len) return su::alloc_copy(TRACEMSG("Null 'json_out_len' parameter specified"));
    try {
        JniCtx& ctx = static_jni_ctx();
        JNIEnv* env = get_jni_env();
        jstring json_ustr = env->NewStringUTF(json_in);
        jobject res = env->CallObjectMethod(ctx.wiltonGatewayObject.get(), ctx.runScriptMethod);
        env->DeleteLocalRef(json_ustr);
        jthrowable exc = env->ExceptionOccurred();
        if (nullptr == exc) {
            std::string res_str = nullptr != res ?
            wj::jstring_to_str(env, static_cast<jstring> (res)) : "";
            *json_out = su::alloc_copy(res_str.c_str());
            *json_out_len = res_str.length();
            return nullptr;
        } else {
            env->ExceptionClear();
            std::string trace = describe_java_exception(env, exc);
            std::string msg = TRACEMSG(trace);
            return su::alloc_copy(msg.c_str());
        }
    } catch (const std::exception& e) {
        return su::alloc_copy(TRACEMSG(e.what() + "\nException raised"));
    }
}

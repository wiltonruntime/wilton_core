/* 
 * File:   wilton_jni.cpp
 * Author: alex
 *
 * Created on January 10, 2017, 6:34 PM
 */

#include "jni.h"

#include <atomic>
#include <memory>
#include <string>

#include "staticlib/config.hpp"
#include "staticlib/io.hpp"
#include "staticlib/utils.hpp"

#include "wilton/wilton.h"
#include "wilton/wiltoncall.h"

#include "common/WiltonInternalException.hpp"
#include "common/utils.hpp"
#include "jni/wilton_jni_config.hpp"

namespace { // anonymous

namespace sc = staticlib::config;
namespace si = staticlib::io;
namespace su = staticlib::utils;
namespace wc = wilton::common;

// non-JNI helpers

std::atomic<bool>& static_jvm_active() {
    static std::atomic<bool> flag{false};
    return flag;
}

// shouldn't be called before logging is initialized by app
void log_error(const std::string& message) {
    static std::string level = "ERROR";
    static std::string logger = "wilton.jni";
    // call wilton
    wilton_logger_log(level.c_str(), level.length(), logger.c_str(), logger.length(),
            message.c_str(), message.length());
}

void dump_startup_error(const std::string& msg) {
    try {
        // err file setup
        auto errdir = std::string(WILTON_STARTUP_ERR_DIR);
        // random postfix
        std::string id = su::RandomStringGenerator().generate(12);
        auto errfile = errdir + "wilton_ERROR_" + id + ".txt";
        auto fd = su::FileDescriptor(errfile, 'w');
        si::write_all(fd, msg);
    } catch (...) {
        // give up
    }
}

// JNI helpers

std::string jstring_to_str(JNIEnv* env, jstring jstr) {
    const char* cstr = env->GetStringUTFChars(jstr, 0);
    size_t cstr_len = static_cast<size_t> (env->GetStringUTFLength(jstr));
    std::string res{cstr, cstr_len};
    env->ReleaseStringUTFChars(jstr, cstr);
    return res;
}

// forward declaration
JNIEnv* get_jni_env();

class GlobalRefDeleter {
public:
    void operator()(jobject ref) {
        if (static_jvm_active().load()) {
            JNIEnv* env = static_cast<JNIEnv*> (get_jni_env());
            env->DeleteGlobalRef(ref);
        }
    }
};

std::unique_ptr<_jclass, GlobalRefDeleter> find_java_class(JNIEnv* env, const std::string& name) {
    jclass local = env->FindClass(name.c_str());
    if (nullptr == local) {
        throw wc::WiltonInternalException(TRACEMSG("Cannot load class, name: [" + name + "]"));
    }
    std::unique_ptr<_jclass, GlobalRefDeleter> res{static_cast<jclass> (env->NewGlobalRef(local)), GlobalRefDeleter()};
    if (nullptr == res.get()) {
        throw wc::WiltonInternalException(TRACEMSG("Cannot create global ref for class, name: [" + name + "]"));
    }
    env->DeleteLocalRef(local);
    return res;
}

jmethodID find_java_method_static(JNIEnv* env, jclass clazz, const std::string& name, const std::string& signature) {
    jmethodID res = env->GetStaticMethodID(clazz, name.c_str(), signature.c_str());
    if (nullptr == res) {
        throw wc::WiltonInternalException(TRACEMSG("Cannot find static method, name: [" + name + "]," +
                " signature: [" + signature + "]"));
    }
    return res;
}

class JniCtx {
public:
    JavaVM* vm;
    std::unique_ptr<_jclass, GlobalRefDeleter> wiltonJniClass;
    jmethodID runScriptMethod;
    jmethodID throwExceptionMethod;
    jmethodID describeExceptionMethod;

    JniCtx(const JniCtx&) = delete;

    JniCtx& operator=(const JniCtx&) = delete;

    JniCtx(JniCtx&&) = delete;

    JniCtx& operator=(JniCtx&& other) {
        this->vm = other.vm;
        other.vm = nullptr;
        this->wiltonJniClass = std::move(other.wiltonJniClass);
        this->runScriptMethod = other.runScriptMethod;
        other.runScriptMethod = nullptr;
        this->throwExceptionMethod = other.throwExceptionMethod;
        other.throwExceptionMethod = nullptr;
        this->describeExceptionMethod = other.describeExceptionMethod;
        other.describeExceptionMethod = nullptr;
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
        // runscript
        this->wiltonJniClass = find_java_class(env, WILTON_JNI_CLASS_SIGNATURE_STR);
        this->runScriptMethod = find_java_method_static(env, this->wiltonJniClass.get(),
                WILTON_JNI_RUNSCRIPT_METHOD_STR, WILTON_JNI_RUNSCRIPT_METHOD_SIGNATURE_STR);
        this->throwExceptionMethod = find_java_method_static(env, this->wiltonJniClass.get(),
                WILTON_JNI_THROWEXCEPTION_METHOD_STR, WILTON_JNI_THROWEXCEPTION_METHOD_SIGNATURE_STR);
        this->describeExceptionMethod = find_java_method_static(env, this->wiltonJniClass.get(),
                WILTON_JNI_DESCRIBEEXCEPTION_METHOD_STR, WILTON_JNI_DESCRIBEEXCEPTION_METHOD_SIGNATURE_STR);
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

void throw_java_exception(JNIEnv* env, const std::string& message) {
    jstring umsg = env->NewStringUTF(message.c_str());
    get_jni_env()->CallStaticVoidMethod(static_jni_ctx().wiltonJniClass.get(),
            static_jni_ctx().throwExceptionMethod, umsg);
    env->DeleteLocalRef(umsg);
}

std::string describe_java_exception(JNIEnv* env, jthrowable exc) {
    jobject umsg = env->CallStaticObjectMethod(static_jni_ctx().wiltonJniClass.get(),
            static_jni_ctx().describeExceptionMethod, exc);
    if (!env->ExceptionOccurred()) {
        std::string res = jstring_to_str(env, static_cast<jstring>(umsg));
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
        // wiltoncalls
        wiltoncall_init();
        // move-assign static ctx
        static_jni_ctx() = JniCtx(vm);
        // set init flag
        static_jvm_active().store(true);
        return WILTON_JNI_VERSION;
    } catch (const std::exception& e) {
        dump_startup_error(TRACEMSG(e.what() + "\nInitialization error"));
        return -1;
    }
}

JNIEXPORT void JNI_OnUnload(JavaVM*, void*) {
    delete std::addressof(static_jni_ctx());
    // flip init flag
    static_jvm_active().store(false);
}

JNIEXPORT jstring JNICALL WILTON_JNI_FUNCTION(wiltoncall)
(JNIEnv* env, jclass, jstring name, jstring data) {
    if (nullptr == name) {
        throw_java_exception(env, TRACEMSG("Null 'name' parameter specified"));
        return nullptr;
    }
    if (nullptr == data) {
        throw_java_exception(env, TRACEMSG("Null 'data' parameter specified"));
        return nullptr;
    }
    std::string name_string = jstring_to_str(env, name);
    std::string data_string = jstring_to_str(env, data);
    char* out;
    int out_len;
    auto err = wiltoncall(name_string.c_str(), static_cast<int>(name_string.length()), 
            data_string.c_str(), static_cast<int>(data_string.length()),
            std::addressof(out), std::addressof(out_len));
    if (nullptr == err) {
        return env->NewStringUTF(out);
    } else {
        std::string msg = TRACEMSG(err +
            "\n'wiltoncall' error for name: [" + name_string + "], data: [" + data_string + "]");
        wilton_free(err);
        throw_java_exception(env, msg);
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
        JNIEnv* env = get_jni_env();
        jstring json_ustr = env->NewStringUTF(json_in);
        jobject res = env->CallStaticObjectMethod(static_jni_ctx().wiltonJniClass.get(),
                static_jni_ctx().runScriptMethod);
        env->DeleteLocalRef(json_ustr);
        jthrowable exc = env->ExceptionOccurred();
        if (nullptr == exc) {
            std::string res_str = nullptr != res ?
            jstring_to_str(env, static_cast<jstring> (res)) : "";
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

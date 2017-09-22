/* 
 * File:   jni_utils.cpp
 * Author: alex
 * 
 * Created on January 12, 2017, 3:12 PM
 */

#include "jni/jni_utils.hpp"

#include "staticlib/config.hpp"

#include "wilton/support/exception.hpp"

namespace wilton {
namespace jni {

std::string jstring_to_str(JNIEnv* env, jstring jstr) {
    const char* cstr = env->GetStringUTFChars(jstr, 0);
    size_t cstr_len = static_cast<size_t> (env->GetStringUTFLength(jstr));
    std::string res{cstr, cstr_len};
    env->ReleaseStringUTFChars(jstr, cstr);
    return res;
}

jclass find_java_class(JNIEnv* env, const std::string& name) {
    jclass local = env->FindClass(name.c_str());
    if (nullptr == local) {
        throw support::exception(TRACEMSG("Cannot load class, name: [" + name + "]"));
    }
    jclass global = static_cast<jclass>(env->NewGlobalRef(local));
    if (nullptr == global) {
        throw support::exception(TRACEMSG("Cannot create global ref for class, name: [" + name + "]"));
    }
    env->DeleteLocalRef(local);
    return global;
}

jmethodID find_java_method(JNIEnv* env, jclass clazz, const std::string& name, const std::string& signature) {
    jmethodID res = env->GetMethodID(clazz, name.c_str(), signature.c_str());
    if (nullptr == res) {
        throw support::exception(TRACEMSG("Cannot find method, name: [" + name + "]," +
                " signature: [" + signature + "]"));
    }
    return res;
}

jmethodID find_java_method_static(JNIEnv* env, jclass clazz, const std::string& name, const std::string& signature) {
    jmethodID res = env->GetStaticMethodID(clazz, name.c_str(), signature.c_str());
    if (nullptr == res) {
        throw support::exception(TRACEMSG("Cannot find static method, name: [" + name + "]," +
                " signature: [" + signature + "]"));
    }
    return res;
}

} // namespace
}

/* 
 * File:   jni_utils.hpp
 * Author: alex
 *
 * Created on January 12, 2017, 3:11 PM
 */

#ifndef WILTON_JNI_JNI_UTILS_HPP
#define	WILTON_JNI_JNI_UTILS_HPP

#include <string>

#include "jni.h"

namespace wilton {
namespace jni {

std::string describe_throwable(JNIEnv* env, jthrowable throwable);

std::string jstring_to_str(JNIEnv* env, jstring jstr);

jclass find_java_class(JNIEnv* env, const std::string& name);

jmethodID find_java_method(JNIEnv* env, jclass clazz, const std::string& name, const std::string& signature);

jmethodID find_java_method_static(JNIEnv* env, jclass clazz, const std::string& name, const std::string& signature);

} // namespace
}

#endif	/* WILTON_JNI_JNI_UTILS_HPP */


# Copyright 2016, alex at staticlibs.net
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required ( VERSION 2.8.12 )

# project
project ( wilton CXX )
set ( ${PROJECT_NAME}_STATICLIB_VERSION 0.8.0 CACHE INTERNAL "" )
set ( ${PROJECT_NAME}_DESCRIPTION "Wilton Web Toolkit library" )
set ( ${PROJECT_NAME}_URL https://github.com/wilton-web-toolkit/libwilton )
include ( ${CMAKE_CURRENT_LIST_DIR}/resources/macros.cmake )

# check deplibs cache
if ( STATICLIB_USE_DEPLIBS_CACHE )
    set ( ${PROJECT_NAME}_CACHED_STATIC_LIB_PATH ${STATICLIB_DEPLIBS_CACHE_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}${PROJECT_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX} )
    set ( ${PROJECT_NAME}_CACHED_SHARED_LIB_PATH ${STATICLIB_DEPLIBS_CACHE_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}${PROJECT_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX} )
    if ( EXISTS ${${PROJECT_NAME}_CACHED_STATIC_LIB_PATH} )
        add_library( ${PROJECT_NAME} STATIC IMPORTED GLOBAL )
        set_target_properties( ${PROJECT_NAME} PROPERTIES IMPORTED_LOCATION ${${PROJECT_NAME}_CACHED_STATIC_LIB_PATH} )
        message ( STATUS "Using cached library: [${${PROJECT_NAME}_CACHED_STATIC_LIB_PATH}]" )
        return ( )
    elseif ( EXISTS ${${PROJECT_NAME}_CACHED_SHARED_LIB_PATH} )
        add_library( ${PROJECT_NAME} SHARED IMPORTED GLOBAL )
        set_target_properties( ${PROJECT_NAME} PROPERTIES IMPORTED_LOCATION ${${PROJECT_NAME}_CACHED_SHARED_LIB_PATH} )
        message ( STATUS "Using cached library: [${${PROJECT_NAME}_CACHED_SHARED_LIB_PATH}]" )
        return ( )
    endif ( )
endif ( )

#options
option ( ${PROJECT_NAME}_SHARED "Build shared library instead of a static one" ON )
if ( NOT DEFINED STATICLIB_DEPS )
    set ( STATICLIB_DEPS ${CMAKE_CURRENT_LIST_DIR}/../../staticlibs/ CACHE INTERNAL "" )    
endif ( )

# dep options
set ( staticlib_pion_USE_LOG4CPLUS ON CACHE BOOL "Use log4cplus lib for logging" )
set ( staticlib_pion_USE_OPENSSL ON CACHE BOOL "Use OpenSSL lib for https" )

# docs
option ( ${PROJECT_NAME}_ENABLE_DOCS "Generate doxyfile and exit build" OFF )
if ( ${PROJECT_NAME}_ENABLE_DOCS )
    configure_file ( ${CMAKE_CURRENT_LIST_DIR}/resources/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile )
    return ( )
endif ( )

# standalone build
if ( NOT DEFINED STATICLIB_TOOLCHAIN )
    if ( NOT DEFINED ${PROJECT_NAME}_TOOLCHAIN )
        if ( WIN32 )
            if ( CMAKE_SIZEOF_VOID_P EQUAL 8 )
                set ( ${PROJECT_NAME}_TOOLCHAIN "windows_amd64_msvc" )
            else ( )
                set ( ${PROJECT_NAME}_TOOLCHAIN "windows_i386_msvc" )
            endif ( )
        elseif ( APPLE )
            set ( ${PROJECT_NAME}_TOOLCHAIN "macosx_amd64_clang" )
        elseif ( UNIX )
            set ( ${PROJECT_NAME}_TOOLCHAIN "linux_amd64_gcc" )
        else ( )
            message ( FATAL_ERROR "Cannot determine toolchain to use, please specify [${PROJECT_NAME}_TOOLCHAIN]" )
        endif ( )
    endif ( )
else ( )
    set ( ${PROJECT_NAME}_TOOLCHAIN ${STATICLIB_TOOLCHAIN} )
endif ( )
if ( NOT DEFINED CMAKE_LIBRARY_OUTPUT_DIRECTORY )
    set ( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )
endif ( )
if ( NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Linux" ) # system dependencies implied on Linux
    wilton_add_subdirectory ( ${STATICLIB_DEPS}/external_asio )
    wilton_add_subdirectory ( ${STATICLIB_DEPS}/external_zlib )
    wilton_add_subdirectory ( ${STATICLIB_DEPS}/external_jansson )
    wilton_add_subdirectory ( ${STATICLIB_DEPS}/external_log4cplus )
    wilton_add_subdirectory ( ${STATICLIB_DEPS}/external_openssl )
    wilton_add_subdirectory ( ${STATICLIB_DEPS}/external_curl )
    wilton_add_subdirectory ( ${STATICLIB_DEPS}/external_sqlite )
    wilton_add_subdirectory ( ${STATICLIB_DEPS}/external_popt )
endif ( )
wilton_add_subdirectory ( ${STATICLIB_DEPS}/external_soci )
wilton_add_subdirectory ( ${STATICLIB_DEPS}/external_duktape )
wilton_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_config )
wilton_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_support )
wilton_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_concurrent )
wilton_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_jni )
wilton_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_io )
wilton_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_ranges )
wilton_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_endian )
wilton_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_utils )
wilton_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_pion )    
wilton_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_json )
wilton_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_pimpl )
wilton_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_tinydir )
wilton_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_unzip )
wilton_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_orm )
wilton_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_http )
wilton_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_cron )
wilton_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_mustache )
if ( ${CMAKE_SYSTEM_NAME} STREQUAL "Windows" )
    wilton_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_winservice )
endif ( )

# dependencies
set ( ${PROJECT_NAME}_DEPS
        staticlib_config
        staticlib_support
        staticlib_io
        staticlib_ranges
        staticlib_jni
        staticlib_pion
        staticlib_json
        staticlib_unzip
        staticlib_orm 
        staticlib_http
        staticlib_utils
        staticlib_cron
        staticlib_mustache
        staticlib_tinydir
        staticlib_pimpl
        popt
        jansson
        log4cplus
        sqlite3
        soci
        duktape
        libcurl
        openssl
        zlib )
if ( ${CMAKE_SYSTEM_NAME} STREQUAL "Windows" )
    list ( APPEND ${PROJECT_NAME}_DEPS staticlib_winservice )
endif ( )
wilton_pkg_check_modules ( ${PROJECT_NAME}_DEPS_PC REQUIRED ${PROJECT_NAME}_DEPS )

# build steps
set ( ${PROJECT_NAME}_SRC )

# core
# call
set ( ${PROJECT_NAME}_SRC_CALL
        ${CMAKE_CURRENT_LIST_DIR}/src/call/wiltoncall.cpp )
list ( APPEND ${PROJECT_NAME}_SRC ${${PROJECT_NAME}_SRC_CALL} )
# common
set ( ${PROJECT_NAME}_SRC_COMMON 
        ${CMAKE_CURRENT_LIST_DIR}/src/common/utils.cpp )
list ( APPEND ${PROJECT_NAME}_SRC ${${PROJECT_NAME}_SRC_COMMON} )
# dyload
set ( ${PROJECT_NAME}_SRC_DYLOAD
        ${CMAKE_CURRENT_LIST_DIR}/src/dyload/wiltoncall_dyload.cpp )
list ( APPEND ${PROJECT_NAME}_SRC ${${PROJECT_NAME}_SRC_DYLOAD} )

# engines
# duktape
set ( ${PROJECT_NAME}_SRC_DUKTAPE
        ${CMAKE_CURRENT_LIST_DIR}/src/duktape/duktape_engine.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/duktape/wiltoncall_duktape.cpp )
list ( APPEND ${PROJECT_NAME}_SRC ${${PROJECT_NAME}_SRC_DUKTAPE} )
# jni
set ( ${PROJECT_NAME}_SRC_JNI
        ${CMAKE_CURRENT_LIST_DIR}/src/jni/jni_utils.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/jni/wiltoncall_jni.cpp )
list ( APPEND ${PROJECT_NAME}_SRC ${${PROJECT_NAME}_SRC_JNI} )

# modules
# client
set ( ${PROJECT_NAME}_SRC_CLIENT 
        ${CMAKE_CURRENT_LIST_DIR}/src/client/wilton_client.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/client/wiltoncall_client.cpp )
list ( APPEND ${PROJECT_NAME}_SRC ${${PROJECT_NAME}_SRC_CLIENT} )
# cron
set ( ${PROJECT_NAME}_SRC_CRON
        ${CMAKE_CURRENT_LIST_DIR}/src/cron/cron_task.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/cron/wilton_cron.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/cron/wiltoncall_cron.cpp )
list ( APPEND ${PROJECT_NAME}_SRC ${${PROJECT_NAME}_SRC_CRON} )
# db
set ( ${PROJECT_NAME}_SRC_DB 
        ${CMAKE_CURRENT_LIST_DIR}/src/db/wilton_db.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/db/wiltoncall_db.cpp )
list ( APPEND ${PROJECT_NAME}_SRC ${${PROJECT_NAME}_SRC_DB} )
# fs
set ( ${PROJECT_NAME}_SRC_FS
        ${CMAKE_CURRENT_LIST_DIR}/src/fs/wiltoncall_fs.cpp )
list ( APPEND ${PROJECT_NAME}_SRC ${${PROJECT_NAME}_SRC_FS} )
# logging
set ( ${PROJECT_NAME}_SRC_LOGGING
        ${CMAKE_CURRENT_LIST_DIR}/src/logging/wilton_logger.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/logging/wilton_logging.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/logging/wiltoncall_logging.cpp )
list ( APPEND ${PROJECT_NAME}_SRC ${${PROJECT_NAME}_SRC_LOGGING} )
# misc
set ( ${PROJECT_NAME}_SRC_MISC
        ${CMAKE_CURRENT_LIST_DIR}/src/misc/tcp_connect_checker.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/misc/wilton_misc.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/misc/wiltoncall_misc.cpp )
list ( APPEND ${PROJECT_NAME}_SRC ${${PROJECT_NAME}_SRC_MISC} )
# mustache
set ( ${PROJECT_NAME}_SRC_MUSTACHE
        ${CMAKE_CURRENT_LIST_DIR}/src/mustache/wilton_mustache.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/mustache/wiltoncall_mustache.cpp )
list ( APPEND ${PROJECT_NAME}_SRC ${${PROJECT_NAME}_SRC_MUSTACHE} )
# mutex
set ( ${PROJECT_NAME}_SRC_MUTEX
        ${CMAKE_CURRENT_LIST_DIR}/src/mutex/waitable_mutex.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/mutex/wilton_mutex.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/mutex/wiltoncall_mutex.cpp )
list ( APPEND ${PROJECT_NAME}_SRC ${${PROJECT_NAME}_SRC_MUTEX} )
# server
set ( ${PROJECT_NAME}_SRC_SERVER
        ${CMAKE_CURRENT_LIST_DIR}/src/server/request.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/server/response_writer.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/server/server.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/server/wilton_server.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/server/wiltoncall_server.cpp )
list ( APPEND ${PROJECT_NAME}_SRC ${${PROJECT_NAME}_SRC_SERVER} )
# shared
set ( ${PROJECT_NAME}_SRC_SHARED 
        ${CMAKE_CURRENT_LIST_DIR}/src/shared/wilton_shared.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/shared/wiltoncall_shared.cpp )
list ( APPEND ${PROJECT_NAME}_SRC ${${PROJECT_NAME}_SRC_SHARED} )
# thread
set ( ${PROJECT_NAME}_SRC_THREAD
        ${CMAKE_CURRENT_LIST_DIR}/src/thread/wilton_thread.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/thread/wiltoncall_thread.cpp )
list ( APPEND ${PROJECT_NAME}_SRC ${${PROJECT_NAME}_SRC_THREAD} )

set ( ${PROJECT_NAME}_HEADERS ${CMAKE_CURRENT_LIST_DIR}/include/wilton/wilton.h )
file ( GLOB_RECURSE ${PROJECT_NAME}_HEADERS_PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/*.hpp )
if ( ${PROJECT_NAME}_SHARED )
    set ( ${PROJECT_NAME}_LIBTYPE SHARED )
else ( )
    set ( ${PROJECT_NAME}_LIBTYPE STATIC )
endif ( )

add_library ( ${PROJECT_NAME} ${${PROJECT_NAME}_LIBTYPE}
        ${${PROJECT_NAME}_SRC} 
        ${${PROJECT_NAME}_HEADERS}
        ${${PROJECT_NAME}_HEADERS_PRIVATE} )

source_group ( "src" FILES ${${PROJECT_NAME}_SRC} )
source_group ( "include" FILES ${${PROJECT_NAME}_HEADERS} )
source_group ( "include_private" FILES ${${PROJECT_NAME}_HEADERS_PRIVATE} )
target_include_directories ( ${PROJECT_NAME} BEFORE PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src/
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${${PROJECT_NAME}_DEPS_PC_INCLUDE_DIRS} )
target_compile_options ( ${PROJECT_NAME} PRIVATE ${${PROJECT_NAME}_DEPS_PC_CFLAGS_OTHER} )
if ( ${PROJECT_NAME}_SHARED )
    target_compile_definitions ( ${PROJECT_NAME} PRIVATE -DWILTON_SHARED )
    target_link_libraries ( ${PROJECT_NAME} ${${PROJECT_NAME}_DEPS_PC_STATIC_LIBRARIES} )
endif ( )

# launchers
if ( ${PROJECT_NAME}_SHARED )
    add_executable ( ${PROJECT_NAME}_cli
            ${CMAKE_CURRENT_LIST_DIR}/src/launcher/cli.cpp )
    target_include_directories ( ${PROJECT_NAME}_cli BEFORE PRIVATE
            ${CMAKE_CURRENT_LIST_DIR}/include
            ${${PROJECT_NAME}_DEPS_PC_INCLUDE_DIRS} )
    target_compile_definitions ( ${PROJECT_NAME}_cli PRIVATE -DWILTON_SHARED -DWILTON_SHARED_IMPORT )
    target_link_libraries ( ${PROJECT_NAME}_cli 
            ${PROJECT_NAME} 
            staticlib_json
            staticlib_tinydir
            staticlib_utils )

    if ( ${CMAKE_SYSTEM_NAME} STREQUAL "Windows" )
        add_executable ( ${PROJECT_NAME}_winservice
                ${CMAKE_CURRENT_LIST_DIR}/src/launcher/winservice.cpp )
        target_include_directories ( ${PROJECT_NAME}_cli BEFORE PRIVATE
                ${CMAKE_CURRENT_LIST_DIR}/include
                ${${PROJECT_NAME}_DEPS_PC_INCLUDE_DIRS} )
        target_compile_definitions ( ${PROJECT_NAME}_winservice PRIVATE -DWILTON_SHARED -DWILTON_SHARED_IMPORT )
        target_link_libraries ( ${PROJECT_NAME}_winservice 
                ${PROJECT_NAME}
                staticlib_winservice
                staticlib_utils )
    endif ( )
endif ( )

# maven install
if ( ${PROJECT_NAME}_TOOLCHAIN MATCHES "android_armeabi_[^_]+" )
    set ( ${PROJECT_NAME}_CLASSIFIER "android_armeabi" CACHE INTERNAL "" )
    set ( ${PROJECT_NAME}_JAR_LIB_DIR "armeabi" CACHE INTERNAL "" )
elseif ( ${PROJECT_NAME}_TOOLCHAIN MATCHES "android_i386_[^_]+" )
    set ( ${PROJECT_NAME}_CLASSIFIER "android_i386" CACHE INTERNAL "" )
    set ( ${PROJECT_NAME}_JAR_LIB_DIR "x86" CACHE INTERNAL "" )
elseif ( ${PROJECT_NAME}_TOOLCHAIN MATCHES "linux_amd64_[^_]+" )
    set ( ${PROJECT_NAME}_CLASSIFIER "linux_amd64" CACHE INTERNAL "" )
elseif ( ${PROJECT_NAME}_TOOLCHAIN MATCHES "windows_amd64_[^_]+" )
    set ( ${PROJECT_NAME}_CLASSIFIER "windows_amd64" CACHE INTERNAL "" )
elseif ( ${PROJECT_NAME}_TOOLCHAIN MATCHES "windows_i386_[^_]+" )
    set ( ${PROJECT_NAME}_CLASSIFIER "windows_i386" CACHE INTERNAL "" )
else ( )
    message ( FATAL_ERROR "Unsupported toolchain: [${PROJECT_NAME}_TOOLCHAIN]" )
endif ( )
string ( SUBSTRING ${CMAKE_SHARED_LIBRARY_SUFFIX} 1 -1 ${PROJECT_NAME}_SHARED_SUFFIX )
string ( SUBSTRING ${CMAKE_STATIC_LIBRARY_SUFFIX} 1 -1 ${PROJECT_NAME}_STATIC_SUFFIX )

add_custom_target ( mvninstall_shared
        COMMAND mvn install:install-file
                -Dfile=${CMAKE_SHARED_LIBRARY_PREFIX}${PROJECT_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX}
                -DgroupId=net.wiltontoolkit
                -DartifactId=${PROJECT_NAME}
                -Dversion=${${PROJECT_NAME}_STATICLIB_VERSION}
                -Dclassifier=${${PROJECT_NAME}_CLASSIFIER}
                -Dpackaging=${${PROJECT_NAME}_SHARED_SUFFIX}
        WORKING_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}        
        COMMENT "Installing shared library into local Maven repository ..." )

add_custom_target ( mvninstall_import
        COMMAND mvn install:install-file
                -Dfile=${CMAKE_SHARED_LIBRARY_PREFIX}${PROJECT_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX}
                -DgroupId=net.wiltontoolkit
                -DartifactId=${PROJECT_NAME}
                -Dversion=${${PROJECT_NAME}_STATICLIB_VERSION}
                -Dclassifier=${${PROJECT_NAME}_CLASSIFIER}
                -Dpackaging=${${PROJECT_NAME}_STATIC_SUFFIX}
        WORKING_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}        
        COMMENT "Installing import library into local Maven repository ..." )

# pkg-config
set ( ${PROJECT_NAME}_PC_CFLAGS "-I${CMAKE_CURRENT_LIST_DIR}/include" )
if ( ${PROJECT_NAME}_SHARED )
    set ( ${PROJECT_NAME}_PC_CFLAGS "${${PROJECT_NAME}_PC_CFLAGS} -DWILTON_SHARED -DWILTON_SHARED_IMPORT" )
endif ( )
set ( ${PROJECT_NAME}_PC_LIBS "-L${CMAKE_LIBRARY_OUTPUT_DIRECTORY} -l${PROJECT_NAME}" )
if ( CMAKE_SYSTEM_NAME STREQUAL "Linux" )
    set ( ${PROJECT_NAME}_PC_LIBS "${${PROJECT_NAME}_PC_LIBS} -lpthread -ldl" )
endif ( )
wilton_list_to_string ( ${PROJECT_NAME}_PC_REQUIRES_PRIVATE "" ${PROJECT_NAME}_DEPS )
configure_file ( ${CMAKE_CURRENT_LIST_DIR}/resources/pkg-config.in 
        ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/pkgconfig/${PROJECT_NAME}.pc )

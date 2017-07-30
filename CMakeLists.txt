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
project ( wilton_core CXX )
set ( ${PROJECT_NAME}_STATICLIB_VERSION 0.8.0 CACHE INTERNAL "" )
set ( ${PROJECT_NAME}_DESCRIPTION "Wilton Core library" )
set ( ${PROJECT_NAME}_URL https://github.com/wilton-web-toolkit/wilton_core )

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
if ( NOT DEFINED STATICLIB_DEPS )
    set ( STATICLIB_DEPS SPECIFY_ME CACHE INTERNAL "" )    
endif ( )
if ( NOT DEFINED CMAKE_LIBRARY_OUTPUT_DIRECTORY )
    set ( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )
endif ( )

# dependencies add
if ( NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Linux" ) # system dependencies implied on Linux
    staticlib_add_subdirectory ( ${STATICLIB_DEPS}/external_asio )
    staticlib_add_subdirectory ( ${STATICLIB_DEPS}/external_zlib )
    staticlib_add_subdirectory ( ${STATICLIB_DEPS}/external_jansson )
    staticlib_add_subdirectory ( ${STATICLIB_DEPS}/external_log4cplus )
    staticlib_add_subdirectory ( ${STATICLIB_DEPS}/external_openssl )
    staticlib_add_subdirectory ( ${STATICLIB_DEPS}/external_curl )
endif ( )
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/external_duktape )
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_config )
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_support )
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_concurrent )
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_jni )
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_io )
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_crypto )
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_ranges )
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_endian )
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_utils )
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_pion )    
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_json )
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_pimpl )
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_tinydir )
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_unzip )
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_http )
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_cron )
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_mustache )

# dependencies check
set ( ${PROJECT_NAME}_DEPS
        staticlib_config
        staticlib_support
        staticlib_io
        staticlib_crypto
        staticlib_ranges
        staticlib_jni
        staticlib_pion
        staticlib_json
        staticlib_unzip
        staticlib_http
        staticlib_utils
        staticlib_cron
        staticlib_mustache
        staticlib_tinydir
        staticlib_pimpl
        jansson
        log4cplus
        duktape
        libcurl
        openssl
        zlib )
staticlib_pkg_check_modules ( ${PROJECT_NAME}_DEPS_PC REQUIRED ${PROJECT_NAME}_DEPS )

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
# load
set ( ${PROJECT_NAME}_SRC_LOAD
        ${CMAKE_CURRENT_LIST_DIR}/src/load/wiltoncall_load.cpp )
list ( APPEND ${PROJECT_NAME}_SRC ${${PROJECT_NAME}_SRC_LOAD} )

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

# library
set ( ${PROJECT_NAME}_RESFILE )
set ( ${PROJECT_NAME}_DEFFILE )
if ( ${CMAKE_SYSTEM_NAME} STREQUAL "Windows" )
    configure_file ( ${CMAKE_CURRENT_LIST_DIR}/resources/${PROJECT_NAME}.rc
            ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.rc )
    set ( ${PROJECT_NAME}_RESFILE ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.rc )
    set ( ${PROJECT_NAME}_DEFFILE ${CMAKE_CURRENT_LIST_DIR}/resources/${PROJECT_NAME}.def )
endif ( )

add_library ( ${PROJECT_NAME} ${${PROJECT_NAME}_LIBTYPE}
        ${${PROJECT_NAME}_SRC} 
        ${${PROJECT_NAME}_HEADERS}
        ${${PROJECT_NAME}_HEADERS_PRIVATE}
        ${${PROJECT_NAME}_RESFILE}
        ${${PROJECT_NAME}_DEFFILE} )

source_group ( "src" FILES ${${PROJECT_NAME}_SRC} )
source_group ( "include" FILES ${${PROJECT_NAME}_HEADERS} )
source_group ( "include_private" FILES ${${PROJECT_NAME}_HEADERS_PRIVATE} )
target_include_directories ( ${PROJECT_NAME} BEFORE PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src/
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${${PROJECT_NAME}_DEPS_PC_INCLUDE_DIRS} )
target_compile_options ( ${PROJECT_NAME} PRIVATE ${${PROJECT_NAME}_DEPS_PC_CFLAGS_OTHER} )
if ( ${PROJECT_NAME}_SHARED )
    target_link_libraries ( ${PROJECT_NAME} PRIVATE ${${PROJECT_NAME}_DEPS_PC_STATIC_LIBRARIES} )
endif ( )

# version file
configure_file ( ${CMAKE_CURRENT_LIST_DIR}/resources/wilton.version
        ${CMAKE_CURRENT_BINARY_DIR}/wilton.version )

# pkg-config
set ( ${PROJECT_NAME}_PC_CFLAGS "-I${CMAKE_CURRENT_LIST_DIR}/include" )
set ( ${PROJECT_NAME}_PC_LIBS "-L${CMAKE_LIBRARY_OUTPUT_DIRECTORY} -l${PROJECT_NAME}" )
if ( CMAKE_SYSTEM_NAME STREQUAL "Linux" )
    set ( ${PROJECT_NAME}_PC_LIBS "${${PROJECT_NAME}_PC_LIBS} -lpthread -ldl" )
endif ( )
staticlib_list_to_string ( ${PROJECT_NAME}_PC_REQUIRES_PRIVATE "" ${PROJECT_NAME}_DEPS )
configure_file ( ${CMAKE_CURRENT_LIST_DIR}/resources/pkg-config.in 
        ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/pkgconfig/${PROJECT_NAME}.pc )

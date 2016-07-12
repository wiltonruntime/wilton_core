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
set ( ${PROJECT_NAME}_STATICLIB_VERSION 1.0 )
set ( ${PROJECT_NAME}_DESCRIPTION "Wilton Web Toolkit C library" )
set ( ${PROJECT_NAME}_URL https://github.com/wilton-web-toolkit/libwilton )
include ( ${CMAKE_CURRENT_LIST_DIR}/resources/macros.cmake )

#options
option ( ${PROJECT_NAME}_SHARED "Build shared library instead of a static one" OFF )
if ( NOT DEFINED STATICLIB_DEPS )
    set ( STATICLIB_DEPS ${CMAKE_CURRENT_LIST_DIR}/../../staticlibs/ CACHE INTERNAL "" )    
endif ( )
option ( STATICLIB_WITH_ICU "Build with ICU support" OFF )

# dep options
set ( staticlib_httpserver_USE_LOG4CPLUS ON CACHE BOOL "Use log4cplus lib for logging" )
set ( staticlib_httpserver_USE_OPENSSL ON CACHE BOOL "Use OpenSSL lib for https" )

# docs
option ( ${PROJECT_NAME}_ENABLE_DOCS "Generate doxyfile and exit build" OFF )
if ( ${PROJECT_NAME}_ENABLE_DOCS )
    configure_file ( ${CMAKE_CURRENT_LIST_DIR}/resources/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile )
    return ( )
endif ( )

# standalone build
if ( NOT DEFINED CMAKE_LIBRARY_OUTPUT_DIRECTORY )
    set ( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )
endif ( )
if ( NOT DEFINED STATICLIB_TOOLCHAIN )
    if ( NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Linux" ) # system dependencies implied on Linux
        wilton_add_subdirectory ( ${STATICLIB_DEPS}/external_zlib )
        wilton_add_subdirectory ( ${STATICLIB_DEPS}/external_jansson )
        wilton_add_subdirectory ( ${STATICLIB_DEPS}/external_log4cplus )
        wilton_add_subdirectory ( ${STATICLIB_DEPS}/external_openssl )
        wilton_add_subdirectory ( ${STATICLIB_DEPS}/external_curl )
        wilton_add_subdirectory ( ${STATICLIB_DEPS}/external_sqlite )
        if ( STATICLIB_WITH_ICU )
            wilton_add_subdirectory ( ${STATICLIB_DEPS}/external_icu )
        endif ( )
    endif ( )
    wilton_add_subdirectory ( ${STATICLIB_DEPS}/external_mstch )
    wilton_add_subdirectory ( ${STATICLIB_DEPS}/external_soci )
    wilton_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_config )
    wilton_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_io )
    wilton_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_ranges )
    wilton_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_endian )
    wilton_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_httpserver )    
    wilton_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_utils )
    if ( STATICLIB_WITH_ICU )
        wilton_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_icu )
    endif ( )
    wilton_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_serialization )
    wilton_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_pimpl )
    wilton_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_unzip )
    wilton_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_orm )
    wilton_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_httpclient )
    wilton_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_cron )
endif ( )

# dependencies
set ( ${PROJECT_NAME}_DEPS
        staticlib_config
        staticlib_httpserver
        staticlib_serialization
        staticlib_unzip
        staticlib_orm 
        staticlib_httpclient
        staticlib_utils
        staticlib_cron
        staticlib_pimpl
        jansson
        log4cplus
        mstch
        sqlite3
        soci
        libcurl
        libssl
        libcrypto
        zlib )
if ( STATICLIB_WITH_ICU )
    set ( ${PROJECT_NAME}_DEPS ${${PROJECT_NAME}_DEPS} 
            icu-uc
            staticlib_icu )
endif ( )
wilton_pkg_check_modules ( ${PROJECT_NAME}_DEPS_PC REQUIRED ${PROJECT_NAME}_DEPS )

# build steps
file ( GLOB_RECURSE ${PROJECT_NAME}_SRC ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp )
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
    target_link_libraries ( ${PROJECT_NAME} ${${PROJECT_NAME}_DEPS_PC_LIBRARIES} )
endif ( )
if ( STATICLIB_WITH_ICU )
    target_compile_definitions ( ${PROJECT_NAME} PRIVATE -DSTATICLIB_WITH_ICU )
endif ( )

# pkg-config
set ( ${PROJECT_NAME}_PC_CFLAGS "-I${CMAKE_CURRENT_LIST_DIR}/include" )
if ( STATICLIB_WITH_ICU )
    set ( ${PROJECT_NAME}_PC_CFLAGS "${${PROJECT_NAME}_PC_CFLAGS} -DSTATICLIB_WITH_ICU" )
endif ( )
if ( ${PROJECT_NAME}_SHARED )
    set ( ${PROJECT_NAME}_PC_CFLAGS "${${PROJECT_NAME}_PC_CFLAGS} -DWILTON_SHARED -DWILTON_SHARED_IMPORT" )
endif ( )
set ( ${PROJECT_NAME}_PC_LIBS "-L${CMAKE_LIBRARY_OUTPUT_DIRECTORY} -l${PROJECT_NAME}" )
if ( CMAKE_SYSTEM_NAME STREQUAL "Linux" )
    set ( ${PROJECT_NAME}_PC_LIBS "${${PROJECT_NAME}_PC_LIBS} -lpthread" )
endif ( )
if ( NOT ${PROJECT_NAME}_SHARED )
    wilton_list_to_string ( ${PROJECT_NAME}_PC_REQUIRES_PRIVATE "" ${PROJECT_NAME}_DEPS )
endif ( )
configure_file ( ${CMAKE_CURRENT_LIST_DIR}/resources/pkg-config.in 
        ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/pkgconfig/${PROJECT_NAME}.pc )

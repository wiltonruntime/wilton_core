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

# options
set ( external_jansson_ENABLE_PRECONFIGURATION ON CACHE BOOL "" )

# dependencies add
if ( STATICLIB_TOOLCHAIN MATCHES "(android|windows|macosx)_.+" )
    staticlib_add_subdirectory ( ${STATICLIB_DEPS}/external_jansson )
endif ( )
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_config )
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_support )
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_concurrent )
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_io )
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_ranges )
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_endian )
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_utils )
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_json )
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_pimpl )
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_tinydir )

# dependencies check
set ( ${PROJECT_NAME}_DEPS
        staticlib_config
        staticlib_support
        staticlib_io
        staticlib_ranges
        staticlib_json
        staticlib_utils
        staticlib_tinydir
        staticlib_pimpl
        jansson )
staticlib_pkg_check_modules ( ${PROJECT_NAME}_DEPS_PC REQUIRED ${PROJECT_NAME}_DEPS )

# build steps
set ( ${PROJECT_NAME}_SRC )

# core
# call
set ( ${PROJECT_NAME}_SRC_CALL
        ${CMAKE_CURRENT_LIST_DIR}/src/call/wiltoncall.cpp )
list ( APPEND ${PROJECT_NAME}_SRC ${${PROJECT_NAME}_SRC_CALL} )

# dyload
set ( ${PROJECT_NAME}_SRC_DYLOAD
        ${CMAKE_CURRENT_LIST_DIR}/src/dyload/wilton_dyload.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/dyload/wiltoncall_dyload.cpp )
list ( APPEND ${PROJECT_NAME}_SRC ${${PROJECT_NAME}_SRC_DYLOAD} )

# misc
set ( ${PROJECT_NAME}_SRC_MISC
        ${CMAKE_CURRENT_LIST_DIR}/src/misc/wilton_misc.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/misc/wiltoncall_misc.cpp )
list ( APPEND ${PROJECT_NAME}_SRC ${${PROJECT_NAME}_SRC_MISC} )

set ( ${PROJECT_NAME}_HEADERS ${CMAKE_CURRENT_LIST_DIR}/include/wilton/wilton.h )
file ( GLOB_RECURSE ${PROJECT_NAME}_HEADERS_PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/*.hpp )

# library
set ( ${PROJECT_NAME}_RESFILE )
set ( ${PROJECT_NAME}_DEFFILE )
set ( ${PROJECT_NAME}_ADDITIONAL_LIBS )
if ( STATICLIB_TOOLCHAIN MATCHES "android_.+" )
    set ( ${PROJECT_NAME}_ADDITIONAL_LIBS dl )
elseif ( STATICLIB_TOOLCHAIN MATCHES "windows_.+" )
    configure_file ( ${WILTON_DIR}/resources/buildres/wilton_module.rc
            ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.rc )
    set ( ${PROJECT_NAME}_RESFILE ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.rc )
    set ( ${PROJECT_NAME}_DEFFILE ${CMAKE_CURRENT_LIST_DIR}/resources/${PROJECT_NAME}.def )
    set ( ${PROJECT_NAME}_ADDITIONAL_LIBS wtsapi32 )
elseif ( STATICLIB_TOOLCHAIN MATCHES "linux_.+" )
    set ( ${PROJECT_NAME}_ADDITIONAL_LIBS pthread dl )
endif ( )

add_library ( ${PROJECT_NAME} SHARED
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

target_link_libraries ( ${PROJECT_NAME} PRIVATE
        ${${PROJECT_NAME}_DEPS_PC_LIBRARIES}
        ${${PROJECT_NAME}_ADDITIONAL_LIBS} )

# platform-specific link options
if ( STATICLIB_TOOLCHAIN MATCHES "android_.+" )
    set_property ( TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY LINK_FLAGS "-Wl,-soname,lib${PROJECT_NAME}.so" )
elseif ( STATICLIB_TOOLCHAIN MATCHES "windows_.+" )
    set_property ( TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY LINK_FLAGS "/manifest:no" )
endif ( )

# debuginfo
staticlib_extract_debuginfo_shared ( ${PROJECT_NAME} )

# pkg-config
set ( ${PROJECT_NAME}_PC_CFLAGS "-I${CMAKE_CURRENT_LIST_DIR}/include" )
set ( ${PROJECT_NAME}_PC_LIBS "-L${CMAKE_LIBRARY_OUTPUT_DIRECTORY} -l${PROJECT_NAME}" )
if ( STATICLIB_TOOLCHAIN MATCHES "linux_.+" )
    set ( ${PROJECT_NAME}_PC_LIBS "${${PROJECT_NAME}_PC_LIBS} -lpthread -ldl" )
endif ( )
staticlib_list_to_string ( ${PROJECT_NAME}_PC_REQUIRES_PRIVATE "" ${PROJECT_NAME}_DEPS )
configure_file ( ${WILTON_DIR}/resources/buildres/pkg-config.in 
        ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/pkgconfig/${PROJECT_NAME}.pc )

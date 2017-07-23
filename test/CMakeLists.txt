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

# toolchain setup
if ( NOT DEFINED STATICLIB_DEPS )
    set ( STATICLIB_DEPS ${CMAKE_CURRENT_LIST_DIR}/../../../staticlibs CACHE INTERNAL "" )    
endif ( )
set ( STATICLIB_TOOLCHAIN linux_amd64_gcc CACHE STRING "toolchain triplet" )
if ( NOT DEFINED STATICLIB_CMAKE )
    set ( STATICLIB_CMAKE ${STATICLIB_DEPS}/cmake CACHE INTERNAL "" )    
endif ( )
set ( CMAKE_TOOLCHAIN_FILE ${STATICLIB_CMAKE}/toolchains/${STATICLIB_TOOLCHAIN}.cmake CACHE INTERNAL "" )

# project
project ( wilton_core_test CXX C )
include ( ${STATICLIB_CMAKE}/staticlibs_common.cmake )
staticlib_enable_deplibs_cache ( )

# options
set ( wilton_SHARED ON CACHE BOOL "Use shared - cannot link to static libwilton from C code" )
set ( staticlib_pion_USE_LOG4CPLUS ON CACHE BOOL "Use log4cplus lib for logging" )
set ( staticlib_pion_USE_OPENSSL ON CACHE BOOL "Use OpenSSL lib for https" )

# dependencies
staticlib_add_subdirectory ( ${CMAKE_CURRENT_LIST_DIR}/../../wilton_core )
set ( ${PROJECT_NAME}_DEPS wilton_core )
staticlib_pkg_check_modules ( ${PROJECT_NAME}_DEPS_PC REQUIRED ${PROJECT_NAME}_DEPS )

# tests
if ( "${STATICLIB_ROOT_SOURCE_DIR}" STREQUAL "${PROJECT_SOURCE_DIR}" )    
    # C
    enable_testing ( )
    add_executable ( wilton_test ${CMAKE_CURRENT_LIST_DIR}/wilton_test.c )
    target_link_libraries ( wilton_test ${${PROJECT_NAME}_DEPS_PC_LIBRARIES} )
    target_include_directories ( wilton_test BEFORE PRIVATE ${${PROJECT_NAME}_DEPS_PC_INCLUDE_DIRS} )
    target_compile_options ( wilton_test PRIVATE ${${PROJECT_NAME}_DEPS_PC_CFLAGS_OTHER} ) 
    set_target_properties ( wilton_test PROPERTIES FOLDER "test" )
    if ( DEFINED CMAKE_MEMORYCHECK_COMMAND )
        add_test ( wilton_test
                ${CMAKE_MEMORYCHECK_COMMAND} ${CMAKE_MEMORYCHECK_COMMAND_OPTIONS} 
                ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/wilton_test )
    else ( )
        add_test ( wilton_test ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/wilton_test )
    endif ( )
    # module
    add_library ( wilton_test_module SHARED ${CMAKE_CURRENT_LIST_DIR}/wilton_test_module.c )
endif ( )

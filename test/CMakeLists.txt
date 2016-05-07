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
set ( STATICLIB_TOOLCHAIN linux_amd64_gcc CACHE STRING "toolchain triplet" )
if ( NOT DEFINED STATICLIB_CMAKE )
    set ( STATICLIB_CMAKE ${CMAKE_CURRENT_LIST_DIR}/../../cmake CACHE INTERNAL "" )    
endif ( )
set ( CMAKE_TOOLCHAIN_FILE ${STATICLIB_CMAKE}/toolchains/${STATICLIB_TOOLCHAIN}.cmake CACHE INTERNAL "" )

# project
project ( wilton_c_test CXX C )
include ( ${STATICLIB_CMAKE}/staticlibs_common.cmake )
staticlib_enable_deplibs_cache ( )

# dependencies
if ( NOT DEFINED STATICLIB_DEPS )
    set ( STATICLIB_DEPS ${CMAKE_CURRENT_LIST_DIR}/../../ CACHE INTERNAL "" )    
endif ( )
if ( NOT STATICLIB_TOOLCHAIN MATCHES "linux_[^_]+_[^_]+" )    
    staticlib_add_subdirectory ( ${STATICLIB_DEPS}/external_jansson )
endif ( )
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_config )
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_io )
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_ranges )
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_httpserver )
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_utils )
staticlib_add_subdirectory ( ${STATICLIB_DEPS}/staticlib_serialization )
staticlib_add_subdirectory ( ${CMAKE_CURRENT_LIST_DIR}/../../wilton_c )
set ( ${PROJECT_NAME}_DEPS wilton_c )
staticlib_pkg_check_modules ( ${PROJECT_NAME}_DEPS_PC REQUIRED ${PROJECT_NAME}_DEPS )

# tests
if ( "${STATICLIB_ROOT_SOURCE_DIR}" STREQUAL "${PROJECT_SOURCE_DIR}" )
    # C
    enable_testing ( )
    add_executable ( wilton_c_test ${CMAKE_CURRENT_LIST_DIR}/wilton_c_test.c )
    target_link_libraries ( wilton_c_test ${${PROJECT_NAME}_DEPS_PC_LIBRARIES} )
    target_include_directories ( wilton_c_test BEFORE PRIVATE ${${PROJECT_NAME}_DEPS_PC_INCLUDE_DIRS} )
    target_compile_options ( wilton_c_test PRIVATE ${${PROJECT_NAME}_DEPS_PC_CFLAGS_OTHER} ) 
    set_target_properties ( wilton_c_test PROPERTIES FOLDER "test" )
    if ( DEFINED CMAKE_MEMORYCHECK_COMMAND )
        add_test ( wilton_c_test
                ${CMAKE_MEMORYCHECK_COMMAND} ${CMAKE_MEMORYCHECK_COMMAND_OPTIONS} 
                ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/wilton_c_test )
    else ( )
        add_test ( wilton_c_test ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/wilton_c_test )
    endif ( )
endif ( )


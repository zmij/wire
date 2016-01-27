#	CMakeLists.txt for boost-process library
#	
#	@author zmij
#	@date Nov 30, 2015

cmake_minimum_required(VERSION 2.6)

# Set library name here
set(lib_name boost-process)
string(TOUPPER ${lib_name} LIB_NAME)
string(REPLACE "-" "_" LIB_NAME ${LIB_NAME})

if (PROJECT_VERSION)
    set(_pversion ${PROJECT_VERSION})
else()
    set(_pversion 0.1.0)
endif()

if (${CMAKE_VERSION} VERSION_GREATER "3.0")
    cmake_policy(SET CMP0048 NEW)
    project(${lib_name} VERSION ${_pversion})
else()
    project(${lib_name})
    set(PROJECT_VERSION ${_pversion})
endif()

# Options
option(BUILD_TESTS "Build tests for the project" ON)
option(BUILD_EXAMPLES "Build example programs for the project" ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_definitions("-std=c++11")

set(${LIB_NAME}_LIB ${lib_name})

add_subdirectory(boost)
if (BUILD_TESTS)
    enable_testing()
    add_subdirectory(libs/process/test)
endif()
if (BUILD_EXAMPLES)
    add_subdirectory(libs/process/example)
endif()

get_directory_property(has_parent PARENT_DIRECTORY)
if (has_parent)
    set(${LIB_NAME}_LIB ${lib_name} CACHE INTERNAL "Name of boost-process library target")
    set(${LIB_NAME}_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "Path to boost-process libaray includes" )
endif()

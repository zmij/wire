#   CMakeLists.txt for testing FindWire.cmake script
#   This directory is NOT included to build tree and is just for testing
#   the FindWire.cmake script
#
#   @author zmij
#   @date Nov 18, 2016

cmake_minimum_required(VERSION 2.6)

set(_pname test-find-wire)
if (PROJECT_VERSION)
    set(_pversion ${PROJECT_VERSION})
else()
    set(_pversion 0.1.0)
endif()

if (${CMAKE_VERSION} VERSION_GREATER "3.0")
    cmake_policy(SET CMP0048 NEW)
    project(${_pname} VERSION ${_pversion})
else()
    project(${_pname})
    set(PROJECT_VERSION ${_pversion})
endif()

add_definitions("-std=c++11")

set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/scripts" )

find_package(Wire REQUIRED)

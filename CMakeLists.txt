#    CMakeLists.txt for wire library
#
#    @author zmij
#    @date Nov 30, 2015

cmake_minimum_required(VERSION 2.6)

# Set library name here
set(lib_name wire)
string(TOUPPER ${lib_name} LIB_NAME)

set(_pname ${lib_name})
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

option(BUILD_TESTS "Build tests for ${lib_name} library" ON)
option(COMPILE_TESTS "Test compile generated sources" OFF)
option(DEBUG_OUTPUT "Generate debug output" OFF)

set( CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules"
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/meta/cmake"
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/afsm/cmake"
)

# wire2cpp generator. Variable is required by wire2cpp cmake script
# When wire is found by find_package, the variable will be set by
# FindWire.cmake script
set(WIRE2CPP wire2cpp)
include(cmake/scripts/wire2cpp.cmake)

set(
    BOOST_COMPONENTS
    system
    thread
    program_options
    filesystem
)
set(BOOST_VERSION 1.58) # We want endian library.

find_package(Boost ${BOOST_VERSION} COMPONENTS ${BOOST_COMPONENTS} REQUIRED)
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(TBB REQUIRED)
find_package(Readline)
find_package(AFSM)

if (Readline_INCLUDE_DIR)
    option(BUILD_CMD_UTILS "Build command line utils" ON)
endif()

if (NOT TIP_IRI_INCLUDE_DIRS)
    set(_TIP_IRI_SUBTREE ON)
endif()

if (NOT METAPUSHKIN_FOUND)
    set(_METAPUSHKIN_SUBTREE ON)
endif()

if (NOT AFSM_FOUND)
    set(_AFSM_SUBTREE ON)
endif()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}/src
    ${Boost_INCLUDE_DIRS}
    ${Readline_INCLUDE_DIR}
    ${TBB_INCLUDE_DIR}
)
wire_include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/idl
)
set(WIRE_IDL_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/idl)
add_definitions("-std=c++11")
add_definitions(-Wall -Werror -pedantic
    -Wno-unused-local-typedefs
    -Wno-unused-parameter
    -Wno-unused-function
    -Wno-unknown-pragmas
)
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
add_definitions(
    -Wno-unused-local-typedef
    -Wno-mismatched-tags
    -Wno-unused-variable
    -Wno-gnu-zero-variadic-macro-arguments
    -Wno-unsequenced
    -Wno-keyword-macro
    -Wno-macro-redefined
)
endif()
if(DEBUG_OUTPUT)
if(NOT WIRE_DEBUG_LEVEL)
    set(WIRE_DEBUG_LEVEL 1)
endif()
add_definitions(-DDEBUG_OUTPUT=${WIRE_DEBUG_LEVEL})
endif()

set(${LIB_NAME}_LIB ${lib_name})
set(${LIB_NAME}_UTIL_LIB ${lib_name}-util)
set(${LIB_NAME}_IFACES_LIB ${lib_name}-ifaces)
set(${LIB_NAME}_IDL_LIB ${lib_name}-idl)
set(${LIB_NAME}_LOCATOR_LIB ${lib_name}-locator)
set(${LIB_NAME}_LOCATOR_BIN ${lib_name}-locatord)

if (BUILD_CMD_UTILS)
set(${LIB_NAME}_CONSOLE_LIB ${lib_name}-console)
endif()

if(_TIP_IRI_SUBTREE)
    add_subdirectory(lib/iri)
endif()

if(_METAPUSHKIN_SUBTREE)
    add_subdirectory(lib/meta)
endif()

if (_AFSM_SUBTREE)
    add_subdirectory(lib/afsm)
endif()

if (NOT BOOST_PROCESS_INCLUDE_DIRS)
    message(STATUS "Use bundled boost::process")
    set(_BP_BUILD_TESTS ${BUILD_TESTS})
    set(BUILD_TESTS OFF)
    option(BUILD_EXAMPLES "" OFF)
    add_subdirectory(lib/process)
    set(BUILD_TESTS ${_BP_BUILD_TESTS})
endif()
message(STATUS "Boost process includes ${BOOST_PROCESS_INCLUDE_DIRS}")

include_directories(
    ${CMAKE_BINARY_DIR}/include
    ${BOOST_PROCESS_INCLUDE_DIRS}
    ${TIP_IRI_INCLUDE_DIRS}
    ${METAPUSHKIN_INCLUDE_DIRS}
    ${AFSM_INCLUDE_DIRS}
)

# Add subdirectories here
add_subdirectory(experiments)
add_subdirectory(src/wire)
add_subdirectory(include)
add_subdirectory(idl)
add_subdirectory(cmake/scripts)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

get_directory_property(has_parent PARENT_DIRECTORY)
if (has_parent)
    set(${LIB_NAME}_LIB ${lib_name} CACHE INTERNAL "Name of wire library target")
    set(${LIB_NAME}_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include CACHE INTERNAL "Path to wire libaray includes" )
    set(WIRE2CPP ${WIRE2CPP} CACHE INTERNAL "wire2cpp program")
endif()

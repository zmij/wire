#	CMakeLists.txt for wire library
#	
#	@author zmij
#	@date Nov 30, 2015

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

set(
    BOOST_COMPONENTS
    system
    thread 
    program_options
)
find_package(Boost 1.58 COMPONENTS ${BOOST_COMPONENTS} REQUIRED)
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${Boost_INCLUDE_DIRS}
)
add_definitions("-std=c++11")

set(${LIB_NAME}_LIB ${lib_name})

# Add subdirectories here
add_subdirectory(experiments)
add_subdirectory(src/wire)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

get_directory_property(has_parent PARENT_DIRECTORY)
if (has_parent)
    set(${LIB_NAME}_LIB ${PROJECT_PREFIX}-${lib_name} CACHE INTERNAL "Name of wire library target")
    set(${LIB_NAME}_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include CACHE INTERNAL "Path to wire libaray includes" )
endif()

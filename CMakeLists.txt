#	CMakeLists.txt for tip-iri library
#	
#	@author zmij
#	@date Nov 30, 2015

cmake_minimum_required(VERSION 2.6)

set(lib_name iri)
string(TOUPPER ${lib_name} LIB_NAME)

if (NOT PROJECT_PREFIX)
    set(PROJECT_PREFIX tip)
endif()
set(_pname ${PROJECT_PREFIX}-${lib_name})
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

# Options
option(BUILD_TESTS "Build tests for the project" ON)

set(BOOST_COMPONENTS
    system
    thread
)

find_package(Boost 1.58 COMPONENTS ${BOOST_COMPONENTS} REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include ${Boost_INCLUDE_DIR})
add_definitions("-std=c++11")

set(TIP_${LIB_NAME}_LIB ${PROJECT_PREFIX}-${lib_name})

add_subdirectory(include/tip)
add_subdirectory(src)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
    set(${LIB_NAME}_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/test)
else()
    set(${LIB_NAME}_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/include)
endif()

get_directory_property(has_parent PARENT_DIRECTORY)
if (has_parent)
    set(TIP_${LIB_NAME}_LIB ${PROJECT_PREFIX}-${lib_name} CACHE INTERNAL "Name of tip-iri library target")
    set(TIP_${LIB_NAME}_INCLUDE_DIRS ${${LIB_NAME}_INCLUDES} CACHE INTERNAL "Path to tip-iri libaray includes" )
endif()


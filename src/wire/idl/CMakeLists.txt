# CMakeLists.txt
# Created on: 25 февр. 2016 г.
#     Author: sergey.fedorov

cmake_minimum_required(VERSION 2.6)

find_program(MCPP mcpp DOC "Portable C preprocessor")
if(NOT MCPP)
    message(FATAL_ERROR "MCPP preprocessor not found. Install MCPP preprocessor package")
endif()

configure_file(config.in.hpp config.hpp)

set(
    BOOST_COMPONENTS
    program_options
    iostreams
)
find_package(Boost ${BOOST_VERSION} COMPONENTS ${BOOST_COMPONENTS} REQUIRED)

set(
    wire_idl_SRCS
    ast.cpp
    qname.cpp
    preprocess.cpp
    parser.cpp
    syntax_error.cpp
)

add_library(${WIRE_IDL_LIB} SHARED ${wire_idl_SRCS})
target_link_libraries(
    ${WIRE_IDL_LIB}
    ${Boost_PROGRAM_OPTIONS_LIBRARIES}
    ${Boost_IOSTREAMS_LIBRARIES}
    ${Boost_SYSTEM_LIBRARIES}
)
set_target_properties(
    ${WIRE_IDL_LIB} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib
)

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
    wire_ast_SRCS
    ast.cpp
    preprocess.cpp
)

add_library(${WIRE_GEN_LIB} SHARED ${wire_ast_SRCS})
target_link_libraries(
    ${WIRE_GEN_LIB}
    ${Boost_PROGRAM_OPTIONS_LIBRARIES}
    ${Boost_IOSTREAMS_LIBRARIES}
    ${Boost_SYSTEM_LIBRARIES}
)
set_target_properties(
    ${WIRE_GEN_LIB} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib
)

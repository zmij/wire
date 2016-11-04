# CMakeLists.txt
# Created on: 24 мая 2016 г.
#     Author: sergey.fedorov

cmake_minimum_required(VERSION 2.6)

set(
    json_lib_SRCS
    json_stream.cpp
    detail/parser_base.cpp
)
add_library(wire-json SHARED ${json_lib_SRCS})
set_target_properties(
    wire-json PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib
)

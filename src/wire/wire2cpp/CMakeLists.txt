# CMakeLists.txt
# Created on: 25 февр. 2016 г.
#     Author: sergey.fedorov

cmake_minimum_required(VERSION 2.6)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

set(
    wire2cpp_SRCS
    wire2cpp.cpp
    cpp_generator.cpp
)

add_executable(wire2cpp ${wire2cpp_SRCS})
target_link_libraries(wire2cpp
    ${Boost_PROGRAM_OPTIONS_LIBRARIES}
    ${Boost_FILESYSTEM_LIBRARIES}
    ${WIRE_IDL_LIB}
    ${WIRE_UTIL_LIB}
)

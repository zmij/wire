# CMakeLists.txt
# Created on: 25 февр. 2016 г.
#     Author: sergey.fedorov

cmake_minimum_required(VERSION 2.6)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

set(
    wire2lua_SRCS
    wire2lua.cpp
    lua_generator.cpp
    lua_source_stream.cpp
)

add_executable(wire2lua ${wire2lua_SRCS})
target_link_libraries(wire2lua
    ${Boost_PROGRAM_OPTIONS_LIBRARIES}
    ${Boost_FILESYSTEM_LIBRARIES}
    ${WIRE_IDL_LIB}
)

install(
    TARGETS wire2lua
    RUNTIME DESTINATION bin
)

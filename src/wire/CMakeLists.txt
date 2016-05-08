#    /tip-proto/src/CMakeLists.txt
#
#    @author zmij
#    @date Dec 11, 2015

cmake_minimum_required(VERSION 2.6)

add_subdirectory(util)

set(wire_lib_SRCS
    encoding/buffer_iterator.cpp
    encoding/buffers.cpp
    encoding/message.cpp
    core/connector.cpp
    core/constants.cpp
    core/endpoint.cpp
    core/transport.cpp
    core/connection.cpp
    core/adapter.cpp
    core/identity.cpp
    core/object.cpp
    core/proxy.cpp
    core/reference.cpp
    core/reference_io.cpp
)
add_library(${WIRE_LIB} SHARED ${wire_lib_SRCS})
set_target_properties(
    ${WIRE_LIB} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib
)
target_link_libraries(
    ${WIRE_LIB}
    ${Boost_PROGRAM_OPTIONS_LIBRARIES}
    ${Boost_SYSTEM_LIBRARIES}
    ${OPENSSL_LIBRARIES}
)

add_subdirectory(idl)
add_subdirectory(wire2cpp)

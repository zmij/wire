#    /tip-proto/src/CMakeLists.txt
#
#    @author zmij
#    @date Dec 11, 2015

cmake_minimum_required(VERSION 2.6)

add_subdirectory(util)
set(
    GENERATED_HEADERS_DIR
    ${CMAKE_BINARY_DIR}/include
)

wire2cpp(
    ${WIRE_IDL_ROOT}/wire/errors/unexpected.wire
    HEADER_DIR      ${GENERATED_HEADERS_DIR}/wire/errors
    INCLUDE_ROOT    wire/errors
    SOURCES         wired_ERRORS
)

wire2cpp(
    ${WIRE_IDL_ROOT}/wire/core/locator.wire
    HEADER_DIR      ${GENERATED_HEADERS_DIR}/wire/core
    INCLUDE_ROOT    wire/core
    SOURCES         wired_LOCATOR
    OPTIONS         --generate-forwards
)

wire2cpp(
    ${WIRE_IDL_ROOT}/wire/core/adapter_admin.wire
    ${WIRE_IDL_ROOT}/wire/core/connector_admin.wire
    HEADER_DIR      ${GENERATED_HEADERS_DIR}/wire/core
    INCLUDE_ROOT    wire/core
    SOURCES         wired_ADMIN
    OPTIONS         --generate-forwards
)

set(wire_lib_SRCS
    ${wired_ERRORS}
    ${wired_LOCATOR}
    ${wired_ADMIN}
    encoding/buffer_sequence.cpp
    encoding/buffers.cpp
    encoding/message.cpp
    errors/exceptions.cpp
    errors/user_exception.cpp
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
    core/adapter_admin_impl.cpp
    core/connector_admin_impl.cpp
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

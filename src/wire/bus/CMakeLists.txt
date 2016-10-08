# CMakeLists.txt
# Created on: 7 окт. 2016 г.
#     Author: sergey.fedorov

cmake_minimum_required(VERSION 2.6)

# Library containing proxy and stub objects for bus service
wire2cpp(
    ${WIRE_IDL_ROOT}/wire/bus/bus.wire
    HEADER_DIR      ${GENERATED_HEADERS_DIR}/wire/bus
    INCLUDE_ROOT    wire/bus
    SOURCES         wired_BUS
    OPTIONS         --generate-forwards
)

set(wire_bus_api_SRCS ${wired_BUS})
add_library(wire-bus-api SHARED ${wire_bus_api_SRCS})
set_target_properties(
    wire-bus-api PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib
)
target_link_libraries(wire-bus-api ${WIRE_LIB})

set(wire_bus_lib_SRCS
    publisher.cpp
    bus_impl.cpp
)
add_library(wire-bus SHARED ${wire_bus_lib_SRCS})
set_target_properties(
    wire-bus PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib
)
target_link_libraries(wire-bus
    ${Boost_THREAD_LIBRARIES}
    ${WIRE_LIB}
    wire-bus-api)

install(
    TARGETS wire-bus-api wire-bus
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

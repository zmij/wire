# CMakeLists.txt
# Created on: 7 окт. 2016 г.
#     Author: sergey.fedorov

cmake_minimum_required(VERSION 2.6)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

# Library containing proxy and stub objects for bus service
wire2cpp(
    ${WIRE_IDL_ROOT}/wire/bus/bus.wire
    HEADER_DIR      ${GENERATED_HEADERS_DIR}/wire/bus
    INCLUDE_ROOT    wire/bus
    SOURCES         wired_BUS
    OPTIONS         --generate-forwards
)

set(wire_bus_api_SRCS ${wired_BUS})
add_library(${WIRE_BUS_API_LIB} SHARED ${wire_bus_api_SRCS})
set_target_properties(
    ${WIRE_BUS_API_LIB} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib
)
target_link_libraries(${WIRE_BUS_API_LIB} ${WIRE_LIB})

set(wire_bus_lib_SRCS
    publisher.cpp
    bus_impl.cpp
    bus_service.cpp
)
add_library(${WIRE_BUS_LIB} SHARED ${wire_bus_lib_SRCS})
set_target_properties(
    ${WIRE_BUS_LIB} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib
)
target_link_libraries(
    ${WIRE_BUS_LIB}
    ${Boost_THREAD_LIBRARIES}
    ${WIRE_LIB}
    ${WIRE_BUS_API_LIB}
)

add_executable(${WIRE_BUS_BIN} bus_main.cpp)
target_link_libraries(
    ${WIRE_BUS_BIN}
    ${WIRE_BUS_LIB}
    ${WIRE_UTIL_LIB}
)

install(
    TARGETS ${WIRE_BUS_API_LIB} ${WIRE_BUS_LIB} ${WIRE_BUS_BIN}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

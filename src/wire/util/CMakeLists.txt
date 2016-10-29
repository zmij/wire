#    /wire/src/wire/util/CMakeLists.txt
#
#    @author zmij
#    @date May 4, 2016

cmake_minimum_required(VERSION 2.6)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

set(wire_util_SRCS
    bits.cpp
    murmur_hash.cpp
    service_runner.cpp
)

add_library(${WIRE_UTIL_LIB} SHARED ${wire_util_SRCS})
target_link_libraries(
    ${WIRE_UTIL_LIB}
    ${Boost_SYSTEM_LIBRARIES}
    ${OPENSSL_LIBRARIES}
)

add_executable(murmur_hash murmur_hash_utility.cpp murmur_hash.cpp)

add_library(${WIRE_IFACES_LIB} SHARED enumerate_interfaces.cpp)
set_target_properties(
    ${WIRE_IFACES_LIB} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib
)
target_link_libraries(
    ${WIRE_IFACES_LIB}
    ${Boost_SYSTEM_LIBRARIES}
    ${OPENSSL_LIBRARIES}
)


add_executable(wire-enum-ifaces enum_ifaces_util.cpp)
target_link_libraries(
    wire-enum-ifaces
    ${WIRE_IFACES_LIB}
    ${Boost_PROGRAM_OPTIONS_LIBRARIES}
)

add_executable(wire-ping wire_ping.cpp)
target_link_libraries(
    wire-ping
    ${WIRE_LIB}
    ${CMAKE_THREAD_LIBS_INIT}
)

if(BUILD_CMD_UTILS)
    set(CONSOLE_LIB ${${LIB_NAME}_CONSOLE_LIB})
    add_library(${CONSOLE_LIB} console.cpp)
    target_link_libraries(${CONSOLE_LIB} ${Readline_LIBRARY})
    set_target_properties(
        ${CONSOLE_LIB} PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION 1
        LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib
    )

    add_executable(wire-cnctr-admin connector_admin.cpp)
    target_link_libraries(
        wire-cnctr-admin
        ${CMAKE_THREAD_LIBS_INIT}
        wire
        ${CONSOLE_LIB}
    )
    install (
        TARGETS ${CONSOLE_LIB} wire-cnctr-admin
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
    )
endif()

install (
    TARGETS ${WIRE_UTIL_LIB} ${WIRE_IFACES_LIB} wire-enum-ifaces
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

#    /wire/test/connector/CMakeLists.txt
#
#    @author zmij
#    @date Mar 1, 2016

cmake_minimum_required(VERSION 2.6)

if (NOT GTEST_INCLUDE_DIRS)
    find_package(GTest REQUIRED)
endif()
if (NOT CMAKE_THREAD_LIBS_INIT)
    find_package(Threads REQUIRED)
endif()

# Add your package dependencies for test here

wire2cpp(
    ping_pong.wire
    HEADER_DIR include/test
    INCLUDE_ROOT test
    OUTPUT wired_SRCS
    #OPTIONS --dont-use-hashed-ids
)

include_directories(${GTEST_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR}/include)

set(
    test_connector_SRCS
    connector_test_main.cpp
    connector_configuration_test.cpp
    ping_pong_test.cpp
    ${wired_SRCS}
)
add_executable(test-wire-connector ${test_connector_SRCS})
target_link_libraries(
    test-wire-connector
    ${GTEST_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${WIRE_LIB}
    sparring-test
)

set(ping_pong_SRCS
    ping_pong_sparring.cpp
    ${wired_SRCS}
    ping_pong_impl.cpp
)

add_executable(ping-pong-sparring ${ping_pong_SRCS})
target_link_libraries(ping-pong-sparring
    ${WIRE_LIB}
)

add_executable(ping-pong-client ping_pong_client.cpp ${wired_SRCS})
target_link_libraries(ping-pong-client
    ${WIRE_LIB}
)

if (GTEST_XML_OUTPUT)
    set (
        TEST_ARGS
        --gtest_output=xml:test-wire-connector-detail.xml
    )
endif()

add_test(
    NAME test-wire-connector
    COMMAND test-wire-connector --sparring-partner $<TARGET_FILE:ping-pong-sparring> ${TEST_ARGS}
)

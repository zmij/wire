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

add_library(wire-test-dummy-gen SHARED dummy_generator_plugin.cpp)
target_link_libraries(wire-test-dummy-gen wire-to-cpp)

wire2cpp(
    ping_pong.wire
    HEADER_DIR include/test
    INCLUDE_ROOT test
    SOURCES wired_SRCS
    OPTIONS --generate-forwards
    PLUGINS $<TARGET_FILE:wire-test-dummy-gen>:make_plugin
)

wire2lua(
    ping_pong.wire
    TARGET ping_pong.lua
)

include_directories(${GTEST_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR}/include)

set(
    test_connector_SRCS
    connector_test_main.cpp
    connector_configuration_test.cpp
    ping_pong_options.cpp
    ping_pong_test.cpp
    ssl_ping_pong_test.cpp
    send_multiple_test.cpp
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
    ${CMAKE_THREAD_LIBS_INIT}
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

if(WITH_BOOST_FIBER)
#-----------------------------------------------------------------------------
#   Test wire core library built with fibers
#-----------------------------------------------------------------------------
set(
    test_fiber_connector_SRCS
    connector_test_main.cpp
    ping_pong_fiber_test.cpp
    ${wired_SRCS}
)
add_executable(test-wire-fiber-connector ${test_fiber_connector_SRCS})
target_link_libraries(
    test-wire-fiber-connector
    ${GTEST_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${WIRE_LIB}
    ${FIBER_LIBRARIES}
    sparring-test
)
target_compile_definitions(test-wire-fiber-connector PRIVATE WITH_BOOST_FIBERS)

if (GTEST_XML_OUTPUT)
    set (
        TEST_ARGS
        --gtest_output=xml:test-wire-fiber-connector-detail.xml
    )
endif()

add_test(
    NAME test-wire-fiber-connector
    COMMAND test-wire-fiber-connector --sparring-partner $<TARGET_FILE:ping-pong-sparring> ${TEST_ARGS}
)

endif()


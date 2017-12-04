# CMakeLists.txt
# Created on: Dec 2, 2017
#     Author: sergey.fedorov

cmake_minimum_required(VERSION 2.6)

include_directories(${GTEST_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})

add_library(test-wire-utils-dummy SHARED dummy_lib.cpp)
get_property(TEST_PLUGIN_PATH TARGET test-wire-utils-dummy PROPERTY LOCATION)

configure_file(plugin_config.in.hpp plugin_config.hpp)

set(test_util_SRCS
    graph_test.cpp
    plugins_test.cpp
)

add_executable(test-wire-utils ${test_util_SRCS})
target_link_libraries(test-wire-utils
    ${GTEST_BOTH_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${WIRE_UTIL_LIB}
)


if (GTEST_XML_OUTPUT)
    set (
        TEST_ARGS
        --gtest_output=xml:test-wire-utils-detail.xml
    )
endif()

add_test(
    NAME test-wire-utils
    COMMAND test-wire-utils ${TEST_ARGS}
)

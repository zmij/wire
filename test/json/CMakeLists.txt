# CMakeLists.txt
# Created on: 22 мая 2016 г.
#     Author: sergey.fedorov

cmake_minimum_required(VERSION 2.6)

find_package(GTest REQUIRED)
set(
    BOOST_COMPONENTS
    program_options
    iostreams
)

find_package(Boost ${BOOST_VERSION} COMPONENTS ${BOOST_COMPONENTS} REQUIRED)

include_directories(${GTEST_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/include)

set(json_test_SRCS
    static_tests.cpp
    parse_tests.cpp
)

add_executable(test-json ${json_test_SRCS})
target_link_libraries(test-json
    ${GTEST_BOTH_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    wire-json
)

if (GTEST_XML_OUTPUT)
    set (
        BASE_TEST_ARGS --gtest_output=xml:test-wire.xml
    )
endif()

add_test(
    NAME test-json
    COMMAND test-json ${BASE_TEST_ARGS}
)

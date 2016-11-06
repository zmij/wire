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
    output_tests.cpp
    parse_tests.cpp
)

add_executable(test-wire-json ${json_test_SRCS})
target_link_libraries(test-wire-json
    ${GTEST_BOTH_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    wire-json
)

if (GTEST_XML_OUTPUT)
    set (
        BASE_TEST_ARGS --gtest_output=xml:test-wire-json.xml
    )
endif()

add_test(
    NAME test-wire-json
    COMMAND test-wire-json ${BASE_TEST_ARGS}
)

if (GBENCH_FOUND)
    set(benchmark_SRCS
        output_benchmarks.cpp
    )
    if (CEREAL_INCLUDE_DIRS)
        include_directories(${CEREAL_INCLUDE_DIRS})
        # Add cereal benchmarks to compare
        message(STATUS "Adding cereal benchmarks")
        list(APPEND benchmark_SRCS cereal_output_benchmarks.cpp)
    endif()
    add_executable(bench-wire-json ${benchmark_SRCS})
    target_link_libraries(bench-wire-json
        ${GBENCH_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        wire-json)
    add_test(
        NAME bench-wire-json
        COMMAND bench-wire-json
    )
endif()

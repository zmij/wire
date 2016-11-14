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
    ${CMAKE_CURRENT_BINARY_DIR})

file(DOWNLOAD https://github.com/nst/JSONTestSuite/archive/master.zip
    ${CMAKE_CURRENT_BINARY_DIR}/json_suite.zip
    SHOW_PROGRESS TLS_VERIFY on
)

execute_process(
    COMMAND ${CMAKE_COMMAND} -E tar xvz json_suite.zip
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    RESULT_VARIABLE result
    OUTPUT_VARIABLE unzip_log
)

if (result EQUAL 0)
    message(STATUS "JSON Test suite downloaded and unzipped successfully")
    string(REGEX MATCH "x[ ]*(JSONTestSuite[^/]*/test_parsing/)[^ ]*\\.json"
        json_tests_root ${unzip_log})
    message(STATUS "Path to json test files is ${CMAKE_MATCH_1}")
    set(JSON_TEST_DATA ${CMAKE_MATCH_1})
    if (JSON_TEST_DATA)
        set(JSON_TEST_DATA "${CMAKE_CURRENT_BINARY_DIR}/${JSON_TEST_DATA}")
        set(RUN_JSONTESTSUITE_TESTS false)
        file(GLOB expected_pass RELATIVE ${JSON_TEST_DATA} ${JSON_TEST_DATA}/y_*.json)
        file(GLOB expected_fail RELATIVE ${JSON_TEST_DATA} ${JSON_TEST_DATA}/n_*.json)
        file(GLOB unsure RELATIVE ${JSON_TEST_DATA} ${JSON_TEST_DATA}/i_*.json)

        set(EXPECTED_PASS)
        foreach(file ${expected_pass})
            if (EXPECTED_PASS)
                set(EXPECTED_PASS "${EXPECTED_PASS}, \"${file}\"")
            else()
                set(EXPECTED_PASS "\"${file}\"")
            endif()
        endforeach()
        set(EXPECTED_FAIL)
        foreach(file ${expected_fail})
            if (EXPECTED_FAIL)
                set(EXPECTED_FAIL "${EXPECTED_FAIL}, \"${file}\"")
            else()
                set(EXPECTED_FAIL "\"${file}\"")
            endif()
        endforeach()
        set(UNSURE_RESULT)
        foreach(file ${expected_fail})
            if (UNSURE_RESULT)
                set(UNSURE_RESULT "${UNSURE_RESULT}, \"${file}\"")
            else()
                set(UNSURE_RESULT "\"${file}\"")
            endif()
        endforeach()

        set(RUN_JSONTESTSUITE_TESTS ON)
    endif()
else()
    set(RUN_JSONTESTSUITE_TESTS OFF)
endif()

set(json_test_SRCS
    static_tests.cpp
    output_tests.cpp
    parse_tests.cpp
)

if (RUN_JSONTESTSUITE_TESTS)
    configure_file(json_test_config.in.hpp json_test_config.hpp)
    list(APPEND json_test_SRCS json_test_suite.cpp)
endif()

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

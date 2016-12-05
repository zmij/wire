# CMakeLists.txt
# Created on: 22 мая 2016 г.
#     Author: sergey.fedorov

cmake_minimum_required(VERSION 2.6)

function(split_list MAX LIST_NUMBERS VAR_NAME)
    set(cnt 0)
    set(lno 0)
    set(current_files)
    set(lists)
    foreach(v ${ARGN})
        if (${cnt} LESS ${MAX})
            math(EXPR cnt "${cnt} + 1")
            list(APPEND current_files ${v})
        else()
            list(APPEND lists "${VAR_NAME}_${lno}")
            set("${VAR_NAME}_${lno}" ${current_files} PARENT_SCOPE)
            math(EXPR lno "${lno} + 1")
            set(cnt 0)
            set(current_files)
        endif()
        message(STATUS "${lno} ${cnt} ${v}")
    endforeach()
    list(APPEND lists "${VAR_NAME}_${lno}")
    set("${VAR_NAME}_${lno}" ${current_files} PARENT_SCOPE)
    set(${LIST_NUMBERS} ${lists} PARENT_SCOPE)
endfunction()

function(instantiate_test_cases TEST_CASE VAR_NAME)
    set(instantiate_text)
    foreach(lst ${ARGN})
        set(current_instantiate "INSTANTIATE_TEST_CASE_P(${lst}, ${TEST_CASE},\n    ::testing::Values(")
        set(cnt 0)
        foreach(file ${${lst}})
            if (cnt GREATER 0)
                set(current_instantiate "${current_instantiate},")
            endif()
            math(EXPR cnt "${cnt}+1")
            set(current_instantiate "${current_instantiate}\n        \"${file}\"")
        endforeach()
        set(current_instantiate "${current_instantiate}));\n\n")
        set(instantiate_text "${instantiate_text}${current_instantiate}")
    endforeach()
    set(${VAR_NAME} ${instantiate_text} PARENT_SCOPE)
endfunction()

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

        split_list(50 __e JTSExpectPass ${expected_pass})
        instantiate_test_cases(pass_parse_test INSTANTIATE_EXPECTED_PASS ${__e})
        split_list(50 __e JTSExpectFail ${expected_fail})
        instantiate_test_cases(fail_parse_test INSTANTIATE_EXPECTED_FAIL ${__e})
        split_list(50 __e JTSUnsure ${unsure})
        instantiate_test_cases(unsure_test INSTANTIATE_UNSURE ${__e})


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
    configure_file(json_test_suite.in.cpp json_test_suite.cpp)
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

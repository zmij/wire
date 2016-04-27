#    /wire/test/idl/CMakeLists.txt
#
#    @author zmij
#    @date Apr 18, 2016

cmake_minimum_required(VERSION 2.6)

if (NOT GTEST_INCLUDE_DIRS)
    find_package(GTest REQUIRED)
endif()
if (NOT CMAKE_THREAD_LIBS_INIT)
    find_package(Threads REQUIRED)
endif()

# Add your package dependencies for test here

include_directories(${GTEST_INCLUDE_DIRS})

set(
    test_program_SRCS
    # Add your sources here
    qname_test.cpp
    ast_test.cpp
    lexer_test.cpp
    lexer_qi_test.cpp
    parser_test.cpp
)
add_executable(test-wire-idl ${test_program_SRCS})
target_link_libraries(
    test-wire-idl
    ${GTEST_BOTH_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    # Add your libraries here
    ${WIRE_IDL_LIB}
)

if (GTEST_XML_OUTPUT)
    set (
        TEST_ARGS
        --gtest_output=xml:test-utils-detail.xml
    )
endif()

add_test(
    NAME test-wire-idl
    COMMAND test-wire-idl ${TEST_ARGS}
)

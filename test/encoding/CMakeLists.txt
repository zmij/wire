#    /tip-proto/test/CMakeLists.txt
#
#    @author zmij
#    @date Dec 10, 2015

cmake_minimum_required(VERSION 2.6)

find_package(GTest REQUIRED)
set(
    BOOST_COMPONENTS
    program_options
    iostreams
)

wire2cpp(
    classes_for_io.wire
    HEADER_DIR include/test
    INCLUDE_ROOT test
    OUTPUT wired_SRCS
    OPTIONS --dont-use-hashed-ids
)

find_package(Boost ${BOOST_VERSION} COMPONENTS ${BOOST_COMPONENTS} REQUIRED)

include_directories(${GTEST_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/include)

set(test_SRCS
    static_tests.cpp
    bits_tests.cpp
    concatenate_test.cpp
    encoding_test.cpp
    varint_io_test.cpp
    variant_io_test.cpp
    message_io_test.cpp
    outgoing_buffer_test.cpp
    endpoints_io_test.cpp
    endpoints_grammar_test.cpp
    uuid_io_test.cpp
    identity_io_test.cpp
    identity_grammar_test.cpp
    containers_io_test.cpp
    optional_io_test.cpp
    segment_io_test.cpp
    exception_io_test.cpp
    reference_grammar_test.cpp
    classes_io_test.cpp
    ${wired_SRCS}
)

add_executable(test-wire-encoding ${test_SRCS})
target_link_libraries(test-wire-encoding
    ${GTEST_BOTH_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${WIRE_LIB}
    ${WIRE_UTIL_LIB}
)

if (GTEST_XML_OUTPUT)
    set (
        BASE_TEST_ARGS --gtest_output=xml:test-wire.xml
    )
endif()

add_test(
    NAME test-wire-encoding
    COMMAND test-wire-encoding ${BASE_TEST_ARGS}
)

add_executable(varint-decode varint-decode.cpp)
target_link_libraries(varint-decode
    ${Boost_PROGRAM_OPTIONS_LIBRARIES}
    ${WIRE_LIB}
)


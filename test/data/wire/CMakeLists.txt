#    /wire/test/data/wire/CMakeLists.txt
#
#    @author zmij
#    @date May 4, 2016

cmake_minimum_required(VERSION 2.6)

file(GLOB WIRE_LIST RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.wire)

wire2cpp(
    ${WIRE_LIST}
    HEADER_DIR include/test
    INCLUDE_ROOT test
    OUTPUT wire_CPPS
)

add_custom_target(
    test_generate_idl ALL
    DEPENDS ${wire_CPPS}
)

foreach(cpp_file ${wire_CPPS})
    string(REPLACE ".cpp" "" test_name ${cpp_file})
    set(test_name "test-compile-generated-${test_name}")
    add_test(
        NAME ${test_name}
        COMMAND ${CMAKE_CXX_COMPILER} -std=c++11
            ${WIRE_INCLUDE_DIRECTORIES} -I${CMAKE_CURRENT_BINARY_DIR}/include
            -c ${cpp_file}
    )
endforeach()

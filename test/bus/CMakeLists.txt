# CMakeLists.txt
# Created on: 16 окт. 2016 г.
#     Author: sergey.fedorov

cmake_minimum_required(VERSION 2.6)

if (NOT GTEST_INCLUDE_DIRS)
    find_package(GTest REQUIRED)
endif()
if (NOT CMAKE_THREAD_LIBS_INIT)
    find_package(Threads REQUIRED)
endif()

# Add your package dependencies for test here

wire2cpp(
    bus_test_interface.wire
    HEADER_DIR include/test
    INCLUDE_ROOT test
    SOURCES wired_SRCS
    OPTIONS --dont-use-hashed-ids
    OPTIONS --generate-forwards
)

include_directories(${GTEST_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR}/include)

set(
    test_bus_SRCS
    # Add your sources here
    bus_test_main.cpp
    bus_inproc_test.cpp
    bus_sparring_test.cpp
    ${wired_SRCS}
)
add_executable(test-wire-bus ${test_bus_SRCS})
target_link_libraries(
    test-wire-bus
    ${GTEST_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    # Add your libraries here
    ${Boost_PROGRAM_OPTIONS}
    ${WIRE_LOCATOR_LIB}
    ${WIRE_BUS_LIB}
    sparring-test
)

if (GTEST_XML_OUTPUT)
    set (
        TEST_ARGS
        --gtest_output=xml:test-utils-detail.xml
    )
endif()

add_test(
    NAME test-wire-bus
    COMMAND test-wire-bus --sparring-partner $<TARGET_FILE:wire-busd> ${TEST_ARGS}
)

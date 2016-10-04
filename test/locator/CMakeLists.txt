#    /wire/test/locator/CMakeLists.txt
#
#    @author zmij
#    @date Sep 29, 2016

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
    test_locator_SRCS
    # Add your sources here
    locator_test_main.cpp
    locator_inproc_test.cpp
    reference_resolve_test.cpp
)
add_executable(test-wire-locator ${test_locator_SRCS})
target_link_libraries(
    test-wire-locator
    ${GTEST_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    # Add your libraries here
    ${Boost_PROGRAM_OPTIONS}
    ${WIRE_LOCATOR_LIB}
)

if (GTEST_XML_OUTPUT)
    set (
        TEST_ARGS
        --gtest_output=xml:test-utils-detail.xml
    )
endif()

add_test(
    NAME test-wire-locator
    COMMAND test-wire-locator ${TEST_ARGS}
)

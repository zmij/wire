#	/test/CMakeLists.txt
#	
#	@author zmij
#	@date Dec 30, 2015

cmake_minimum_required(VERSION 2.6)

if (NOT GTEST_INCLUDE_DIRS)
    find_package(GTest REQUIRED)
endif()
if (NOT CMAKE_THREAD_LIBS_INIT)
    find_package(Threads REQUIRED)
endif()

include_directories(${GTEST_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(iri_test_SRCS
    iri_gen_test.cpp
    iri_parse_test.cpp
    iri_path_test.cpp
)

add_executable(test-iri ${iri_test_SRCS})
target_link_libraries(
    test-iri
    ${GTEST_BOTH_LIBRARIES}
    ${TIP_IRI_LIB}
    ${CMAKE_THREAD_LIBS_INIT}
)

if (GTEST_XML_OUTPUT)
    set (
        TEST_ARGS
        --gtest_output=xml:test-utils-detail.xml
    )
endif()

add_test(
    NAME test-iri
    COMMAND test-iri ${TEST_ARGS}
)

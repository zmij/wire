#	/wire/test/connector/CMakeLists.txt
#	
#	@author zmij
#	@date Mar 1, 2016

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
	test_connector_SRCS
	connector_configuration_test.cpp
)
add_executable(test-wire-connector ${test_connector_SRCS})
target_link_libraries(
	test-wire-connector
	${GTEST_BOTH_LIBRARIES}
	${CMAKE_THREAD_LIBS_INIT}
	${WIRE_LIB}
)

if (GTEST_XML_OUTPUT)
    set (
        TEST_ARGS
        --gtest_output=xml:test-wire-connector-detail.xml
    )
endif()

add_test(
    NAME test-wire-connector
    COMMAND test-wire-connector ${TEST_ARGS}
)

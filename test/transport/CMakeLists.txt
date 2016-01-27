#	/wire/test/transport/CMakeLists.txt
#	
#	@author zmij
#	@date Jan 27, 2016

cmake_minimum_required(VERSION 2.6)

if (NOT GTEST_INCLUDE_DIRS)
    find_package(GTest REQUIRED)
endif()
if (NOT CMAKE_THREAD_LIBS_INIT)
    find_package(Threads REQUIRED)
endif()

# Add your package dependencies for test here
set(
    BOOST_COMPONENTS
    program_options
    iostreams
)
find_package(Boost 1.55 COMPONENTS ${BOOST_COMPONENTS} REQUIRED)

include_directories(${GTEST_INCLUDE_DIRS})

set(test_sparring_SRCS
    sparring_partner.cpp
    tcp_sparring.cpp
)
add_executable(transport-sparring ${test_sparring_SRCS})
target_link_libraries(transport-sparring
    ${Boost_PROGRAM_OPTIONS_LIBRARIES}
    ${Boost_SYSTEM_LIBRARIES}
    ${WIRE_LIB}
)

set(test_transport_SRCS
    transport_test_main.cpp
    options.cpp
    sparring_test.cpp
    tcp_transport_test.cpp
)

add_executable(test-wire-transport ${test_transport_SRCS})
target_link_libraries(test-wire-transport
    ${GTEST_LIBRARIES}
    ${Boost_PROGRAM_OPTIONS_LIBRARIES}
    ${Boost_IOSTREAMS_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${WIRE_LIB}
)

if (GTEST_XML_OUTPUT)
    set (
        TEST_ARGS
        --gtest_output=xml:test-wire-transport-detail.xml
    )
endif()

add_test(
    NAME test-wire-transport
    COMMAND test-wire-transport --sparring-partner $<TARGET_FILE:transport-sparring> ${TEST_ARGS}
)


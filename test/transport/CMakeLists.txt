#    /wire/test/transport/CMakeLists.txt
#
#    @author zmij
#    @date Jan 27, 2016

cmake_minimum_required(VERSION 2.6)

set(test_sparring_SRCS
    sparring_partner.cpp
    sparring_options.cpp
    tcp_sparring.cpp
    tcp_sparring_client.cpp
    ssl_sparring.cpp
    ssl_sparring_client.cpp
    udp_sparring.cpp
    socket_sparring.cpp
)
add_executable(transport-sparring ${test_sparring_SRCS})
target_link_libraries(transport-sparring
    ${Boost_PROGRAM_OPTIONS_LIBRARIES}
    ${Boost_SYSTEM_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${OPENSSL_LIBRARIES}
    ${WIRE_LIB}
)

set(test_transport_SRCS
    transport_test_main.cpp
    tcp_transport_test.cpp
    tcp_transport_server_test.cpp
    ssl_transport_test.cpp
    ssl_transport_server_test.cpp
    udp_transport_test.cpp
    socket_transport_test.cpp
    client_connection_test.cpp
)

add_executable(test-wire-transport ${test_transport_SRCS})
target_link_libraries(test-wire-transport
    ${GTEST_LIBRARIES}
    ${Boost_PROGRAM_OPTIONS_LIBRARIES}
    ${Boost_IOSTREAMS_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${WIRE_LIB}
    sparring-test
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


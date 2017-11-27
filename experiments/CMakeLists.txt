#    /tip-proto/experiments/CMakeLists.txt
#
#    @author zmij
#    @date Dec 10, 2015

cmake_minimum_required(VERSION 2.6)

set(
    BOOST_COMPONENTS
    program_options
    iostreams
)
find_package(Boost ${BOOST_VERSION} COMPONENTS ${BOOST_COMPONENTS} REQUIRED)

set(client_SRCS
    ssl-client.cpp
)
set(client_LIBS
    ${CMAKE_THREAD_LIBS_INIT}
    ${Boost_SYSTEM_LIBRARIES}
    ${Boost_PROGRAM_OPTIONS_LIBRARIES}
    ${OPENSSL_LIBRARIES}
)

add_executable(ssl-client ${client_SRCS})
target_link_libraries(ssl-client ${client_LIBS})

set(server_SRCS
    ssl-server.cpp
)

set(server_LIBS
    ${CMAKE_THREAD_LIBS_INIT}
    ${Boost_SYSTEM_LIBRARIES}
    ${Boost_PROGRAM_OPTIONS_LIBRARIES}
    ${OPENSSL_LIBRARIES}
)

add_executable(ssl-server ${server_SRCS})
target_link_libraries(ssl-server ${server_LIBS})

add_executable(bits-fun bits-fun.cpp)
add_executable(enum-iface enumerate_ifaces.cpp)
target_link_libraries(enum-iface ${client_LIBS})

if(WITH_BOOST_FIBER)
set(fiber_LIBS
    ${CMAKE_THREAD_LIBS_INIT}
    ${Boost_SYSTEM_LIBRARIES}
    ${Boost_CONTEXT_LIBRARIES}
    ${Boost_FIBER_LIBRARIES}
)

add_executable(autoecho autoecho.cpp)
target_link_libraries( autoecho ${fiber_LIBS} )

add_executable(exchange exchange.cpp)
target_link_libraries( exchange ${fiber_LIBS} )

endif()

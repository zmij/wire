#	/tip-proto/experiments/CMakeLists.txt
#	
#	@author zmij
#	@date Dec 10, 2015

cmake_minimum_required(VERSION 2.6)

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


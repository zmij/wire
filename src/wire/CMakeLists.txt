#	/tip-proto/src/CMakeLists.txt
#	
#	@author zmij
#	@date Dec 11, 2015

cmake_minimum_required(VERSION 2.6)

set(wire_lib_SRCS
    bits.cpp
    encoding/buffer_iterator.cpp
    encoding/buffers.cpp
    encoding/message.cpp
    core/constants.cpp
    core/endpoint.cpp
    core/transport.cpp
    core/connection.cpp
    core/proxy.cpp
)
add_library(${WIRE_LIB} SHARED ${wire_lib_SRCS})
set_target_properties(
    ${WIRE_LIB} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)
target_link_libraries(
    ${WIRE_LIB}
    ${Boost_SYSTEM_LIBRARIES}
    ${OPENSSL_LIBRARIES}
)

#	/tip-proto/src/CMakeLists.txt
#	
#	@author zmij
#	@date Dec 11, 2015

cmake_minimum_required(VERSION 2.6)

set(wire_lib_SRCS
    bits.cpp
    encoding/buffers.cpp
)
add_library(${WIRE_LIB} ${wire_lib_SRCS})

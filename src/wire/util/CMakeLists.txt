#    /wire/src/wire/util/CMakeLists.txt
#
#    @author zmij
#    @date May 4, 2016

cmake_minimum_required(VERSION 2.6)

set(wire_util_SRCS
    bits.cpp
    murmur_hash.cpp
)

add_library(${WIRE_UTIL_LIB} ${wire_util_SRCS})

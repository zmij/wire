#    /wire/src/wire/util/CMakeLists.txt
#
#    @author zmij
#    @date May 4, 2016

cmake_minimum_required(VERSION 2.6)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

set(wire_util_SRCS
    bits.cpp
    murmur_hash.cpp
)

add_library(${WIRE_UTIL_LIB} SHARED ${wire_util_SRCS})

add_executable(murmur_hash murmur_hash_utility.cpp murmur_hash.cpp)


if(BUILD_CMD_UTILS)
    set(CONSOLE_LIB ${${LIB_NAME}_CONSOLE_LIB})
    add_library(${CONSOLE_LIB} console.cpp)
    target_link_libraries(${CONSOLE_LIB} ${Readline_LIBRARY})
    set_target_properties(
        ${CONSOLE_LIB} PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION 1
        LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib
    )

    add_executable(wire-cnctr-admin connector_admin.cpp)
    target_link_libraries(
        wire-cnctr-admin
        wire
        ${CONSOLE_LIB}
    )
endif()

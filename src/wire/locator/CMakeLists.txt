#    /wire/src/wire/locator/CMakeLists.txt
#
#    @author zmij
#    @date Sep 27, 2016

cmake_minimum_required(VERSION 2.6)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

# Locator implemetation library
set(
    wire_locator_lib_SRCS
    locator_impl.cpp
    locator_service.cpp
)
add_library(${WIRE_LOCATOR_LIB} SHARED ${wire_locator_lib_SRCS})
set_target_properties(
    ${WIRE_LOCATOR_LIB} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib
)
target_link_libraries(
    ${WIRE_LOCATOR_LIB}
    ${WIRE_LIB}
)

# Locator standalone daemon
set(
    wire_locator_daemon_SRCS
    locator_main.cpp
)

add_executable(${WIRE_LOCATOR_BIN} ${wire_locator_daemon_SRCS})
target_link_libraries(
    ${WIRE_LOCATOR_BIN}
    ${WIRE_LOCATOR_LIB}
    ${WIRE_UTIL_LIB}
)

install(
    TARGETS ${WIRE_LOCATOR_LIB} ${WIRE_LOCATOR_BIN}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

#	/wire/support/CMakeLists.txt
#	
#	@author zmij
#	@date Nov 16, 2017

cmake_minimum_required(VERSION 2.6)

if (UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

if (LINUX)

set(WIRESHARK_ROOT_DIR "$ENV{HOME}/.wireshark")

message(STATUS ">>> ${WIRESHARK_ROOT_DIR}")

set(LIB_FILES wire-core.lua)
set(DISSECTOR wire-protocol.lua)

install(
    FILES ${LIB_FILES}
    DESTINATION "${WIRESHARK_ROOT_DIR}/wire_plugins"
)

install(
    FILES ${DISSECTOR}
    DESTINATION "${WIRESHARK_ROOT_DIR}/plugins"
)

endif()
#	/wire/cmake/scripts/wire2lua.cmake
#	
#	@author zmij
#	@date Nov 23, 2017

if (UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()
if (LINUX)
set(WIRESHARK_ROOT_DIR "$ENV{HOME}/.wireshark")
endif()

## wire2lua function
# Generates commands for generating Lua Wireshark dissector plugins from wire
# id files.
# 
function(wire2lua)
    set(argnames
        OUTPUT
        OPTIONS
    )
    parse_argn("" argnames ${ARGN})
    set(wire2lua_options ${WIRE_IDL_DIRECTORIES})
    if(NOT OUTPUT)
        message(FATAL_ERROR "No output specified for wire2lua")
    endif()
    add_custom_target(
        ${OUTPUT} ALL
        COMMAND ${WIRE2LUA} ${wire2lua_options} --output=${OUTPUT} ${DEFAULT_ARGS}
        DEPENDS ${DEFAULT_ARGS} ${WIRE2LUA}
        COMMENT "Generate Wireshark dissector plugin ${OUTPUT}"
    )
    if(LINUX)
        install(
            FILES ${OUTPUT}
            DESTINATION "${WIRESHARK_ROOT_DIR}/wire_plugins"
        )
    endif()
endfunction()

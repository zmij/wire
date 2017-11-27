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
        TARGET
        OPTIONS
    )
    parse_argn("" argnames ${ARGN})
    set(wire2lua_options ${WIRE_IDL_DIRECTORIES})
    if(NOT TARGET)
        message(FATAL_ERROR "No output specified for wire2lua")
    endif()
    message(STATUS "Add wire2lua target ${TARGET}")
    set(files)
    foreach(wire_file ${DEFAULT_ARGS})
        get_filename_component(base_dir ${wire_file} DIRECTORY)
        if (NOT base_dir)
            set(base_dir ${CMAKE_CURRENT_SOURCE_DIR})
        endif()
        get_filename_component(wire_file ${wire_file} NAME)
        list(APPEND files ${base_dir}/${wire_file})
    endforeach()
    add_custom_target(
        ${TARGET} ALL
        COMMAND ${WIRE2LUA} ${wire2lua_options} --output=${TARGET} ${files}
        DEPENDS ${files} ${WIRE2LUA}
        COMMENT "Generate Wireshark dissector plugin ${OUTPUT}"
    )
    if(LINUX)
        install(
            FILES ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}
            DESTINATION "${WIRESHARK_ROOT_DIR}/wire_plugins"
        )
    endif()
endfunction()

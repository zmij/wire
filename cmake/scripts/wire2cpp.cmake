#    /wire/cmake/scripts/wire2cpp.cmake
#
#    @author zmij
#    @date May 4, 2016

function(parse_argn _PREFIX _ARGNAMES)
    set(current_arg_name ${_PREFIX}DEFAULT_ARGS)
    foreach (arg ${ARGN})
        list(FIND ${_ARGNAMES} ${arg} idx)
        if (NOT idx LESS 0)
            set(current_arg_name ${_PREFIX}${arg})
        else()
            list(APPEND ${current_arg_name} ${arg})
        endif()
    endforeach()

    foreach (arg_name ${${_ARGNAMES}})
        set(${_PREFIX}${arg_name} ${${_PREFIX}${arg_name}} PARENT_SCOPE)
    endforeach()
    if (${_PREFIX}DEFAULT_ARGS)
        set(${_PREFIX}DEFAULT_ARGS ${${_PREFIX}DEFAULT_ARGS} PARENT_SCOPE)
    endif()
endfunction()

## wire_include_directories
# Works like plain include_directories cmake function but affects the include
# paths for wire2cpp generator.
# Sets variable WIRE_IDL_DIRECTORIES
function(wire_include_directories)
    set(wire_includes ${WIRE_IDL_DIRECTORIES})
    foreach(arg ${ARGN})
        list(APPEND wire_includes -I${arg})
    endforeach()
    set(WIRE_IDL_DIRECTORIES ${wire_includes} PARENT_SCOPE)
    set_property(DIRECTORY APPEND PROPERTY WIRE_IDL_DIRECTORIES ${wire_includes})
endfunction()

## wire2cpp macro
# Generates commands for generating C++ header and source files from wire
# idl files.
# INCLUDE_ROOT All includes with files from the same directory will be
#              prepended with this path
# HEADER_DIR   Directory to place header files
# SOURCE_DIR   Directory to place source files
# SOURCES      List of generated source files
# HEADERS      List of generated header files
# DEPENDENCIES List of generated dependency targets
# OPTIONS      Additional options to pass to wire2cpp generator program
function(wire2cpp)
    set(argnames
        INCLUDE_ROOT
        HEADER_DIR
        SOURCE_DIR
        SOURCES
        HEADERS
        DEPENDENCIES
        OPTIONS
        PLUGINS
        DEPENDS
    )
    parse_argn("" argnames ${ARGN})
    set(out_cpps ${${SOURCES}})
    set(out_hpps ${${HEADERS}})
    set(deps)
    set(wire2cpp_options ${WIRE_IDL_DIRECTORIES})

    if(HEADER_DIR)
        list(APPEND wire2cpp_options --header-output-dir=${HEADER_DIR})
    endif()
    if(SOURCE_DIR)
        list(APPEND wire2cpp_options --cpp-output-dir=${SOURCE_DIR})
    endif()
    if(INCLUDE_ROOT)
        list(APPEND wire2cpp_options --header-include-root=${INCLUDE_ROOT})
    endif()
    if(OPTIONS)
        list(APPEND wire2cpp_options ${OPTIONS})
    endif()
    foreach(plugin ${PLUGINS})
        list(APPEND wire2cpp_options --generate-plugin=${plugin})
    endforeach()
    foreach(wire_file ${DEFAULT_ARGS})
        get_filename_component(base_dir ${wire_file} DIRECTORY)
        if (NOT base_dir)
            set(base_dir ${CMAKE_CURRENT_SOURCE_DIR})
        endif()
        get_filename_component(wire_file ${wire_file} NAME)
        string(REPLACE ".wire" ".cpp" cpp_file ${wire_file})
        string(REPLACE ".wire" ".hpp" hpp_file ${wire_file})

        if (SOURCE_DIR)
            set(cpp_file "${SOURCE_DIR}/${cpp_file}")
        endif()
        if (HEADER_DIR)
            set(hpp_file "${HEADER_DIR}/${hpp_file}")
        endif()

        list(APPEND out_cpps ${cpp_file})
        list(APPEND out_hpps ${hpp_file})
        message(STATUS "Add target ${wire_file} -> ${cpp_file}")
        add_custom_command(
            OUTPUT ${cpp_file} ${hpp_file}
            COMMAND ${WIRE2CPP} ${wire2cpp_options} ${base_dir}/${wire_file}
            DEPENDS ${base_dir}/${wire_file} ${WIRE2CPP} ${DEPENDS}
            COMMENT "Generate C++ sources from ${wire_file}"
        )
        if (DEPENDENCIES)
            get_filename_component(target_name ${wire_file} NAME)
            set(target_name ".${target_name}")
            add_custom_target(
                ${target_name}
                COMMAND ${CMAKE_COMMAND} -E touch ${target_name}
                DEPENDS ${cpp_file} ${hpp_file})
            list(APPEND deps ${target_name})
        endif()
    endforeach()
    if (DEPENDENCIES)
        set(${DEPENDENCIES} ${deps} PARENT_SCOPE)
    endif()
    if (SOURCES)
        set(${SOURCES} ${out_cpps} PARENT_SCOPE)
    endif()
    if (HEADERS)
        set(${HEADERS} ${out_hpps} PARENT_SCOPE)
    endif()
endfunction()

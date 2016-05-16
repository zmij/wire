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

function(wire_include_directories)
    set(wire_includes ${WIRE_INCLUDE_DIRECTORIES})
    foreach(arg ${ARGN})
        list(APPEND wire_includes -I${arg})
    endforeach()
    set(WIRE_INCLUDE_DIRECTORIES ${wire_includes} PARENT_SCOPE)
endfunction()

function(wire2cpp)
    set(argnames INCLUDE_ROOT HEADER_DIR SOURCE_DIR OUTPUT OPTIONS)
    parse_argn("" argnames ${ARGN})
    set(out_cpps ${${OUTPUT}})
    set(wire2cpp_options ${WIRE_INCLUDE_DIRECTORIES})
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
        message(STATUS "Add target ${wire_file} -> ${cpp_file}")
        add_custom_command(
            OUTPUT ${cpp_file} ${hpp_file}
            COMMAND wire2cpp ${wire2cpp_options} ${base_dir}/${wire_file}
            DEPENDS ${base_dir}/${wire_file} wire2cpp
            COMMENT "Generate C++ sources from ${wire_file}"
        )
    endforeach()
    if (OUTPUT)
        set(${OUTPUT} ${out_cpps} PARENT_SCOPE)
    endif()
endfunction()

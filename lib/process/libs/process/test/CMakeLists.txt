#	/boost-process/libs/process/test/CMakeLists.txt
#	
#	@author zmij
#	@date Jan 14, 2016

cmake_minimum_required(VERSION 2.6)

set(BOOST_COMPONENTS
    system
    thread
    filesystem
    program_options
    iostreams
    unit_test_framework
)
find_package(Boost 1.55 COMPONENTS ${BOOST_COMPONENTS} REQUIRED)
find_package (Threads REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

add_executable(sparring_partner sparring_partner.cpp)
target_link_libraries(sparring_partner
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_PROGRAM_OPTIONS_LIBRARIES}
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_IOSTREAMS_LIBRARY}
)

set(
    test_SRCS
    bind_stderr.cpp
    bind_stdin_stdout.cpp
    bind_stdin.cpp
    bind_stdout_stderr.cpp
    bind_stdout.cpp
    close_stderr.cpp
    close_stdin.cpp
    close_stdout.cpp
    exit_code.cpp
    extensions.cpp
    inherit_env.cpp
    run_exe.cpp
    run_exe_path.cpp
    run_exe_wstring.cpp
    search_path.cpp
    set_args.cpp
    set_cmd_line.cpp
    set_env.cpp
    set_on_error.cpp
    shell_path.cpp
    start_in_dir.cpp
    terminate.cpp
    throw_on_error.cpp
    wait.cpp
)

# Platform-specific tests
if (UNIX)
    set(test_SRCS ${test_SRCS} posix_specific.cpp)
elseif(WIN32)
    set(test_SRCS ${test_SRCS}
        show_window.cpp 
        windows_specific.cpp
    )
endif()

set (Boost_USE_STATIC_LIBS ON)

foreach(source_file ${test_SRCS})
    string(REPLACE ".cpp" "" target_name ${source_file})
    set(target_name "test_${target_name}")
    add_executable(${target_name} ${source_file})
    target_link_libraries(${target_name} ${Boost_LIBRARIES})
    add_test(NAME ${target_name} COMMAND ${target_name} $<TARGET_FILE:sparring_partner>)
endforeach() 


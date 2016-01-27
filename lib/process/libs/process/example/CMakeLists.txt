#	/boost-process/libs/process/example/CMakeLists.txt
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
)
find_package(Boost 1.58 COMPONENTS ${BOOST_COMPONENTS} REQUIRED)

set(
    example_SRCS
    args.cpp
    async_io.cpp
    cleanup.cpp    
    cmd_line.cpp
    env.cpp
    error_handling.cpp
    execute.cpp
    intro.cpp
    streams.cpp
    sync_io.cpp
    terminate.cpp
    wait.cpp
    work_dir.cpp
)

# Platform-specific tests
if (UNIX)
    set(example_SRCS ${example_SRCS} posix.cpp)
elseif(WIN32)
    set(example_SRCS ${example_SRCS}
        windows.cpp
        windows_unicode.cpp
    )
endif()

foreach(source_file ${example_SRCS})
    string(REPLACE ".cpp" "" target_name ${source_file})
    add_executable(${target_name} ${source_file})
    target_link_libraries(${target_name} ${Boost_LIBRARIES})
endforeach()

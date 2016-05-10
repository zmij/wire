# CMakeLists.txt
# Created on: 30 янв. 2016 г.
#     Author: sergey.fedorov

configure_file(config.in.hpp config.hpp)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(
    BOOST_COMPONENTS
    program_options
    iostreams
)
find_package(Boost ${BOOST_VERSION} COMPONENTS ${BOOST_COMPONENTS} REQUIRED)

if (NOT GTEST_INCLUDE_DIRS)
    find_package(GTest REQUIRED)
endif()
if (NOT CMAKE_THREAD_LIBS_INIT)
    find_package(Threads REQUIRED)
endif()

include_directories(${GTEST_INCLUDE_DIRS})

add_subdirectory(data)
add_subdirectory(idl)
add_subdirectory(encoding)
add_subdirectory(sparring)
add_subdirectory(transport)
add_subdirectory(connector)
add_subdirectory(data/wire)

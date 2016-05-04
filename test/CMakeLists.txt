# CMakeLists.txt
# Created on: 30 янв. 2016 г.
#     Author: sergey.fedorov

configure_file(config.in.hpp config.hpp)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(data)
add_subdirectory(encoding)
add_subdirectory(transport)
add_subdirectory(connector)
add_subdirectory(idl)
add_subdirectory(data/wire)

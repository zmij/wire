# /wire/cmake/scripts/FindWire.cmake
# Try to find wire library
#
# The following variables are optionally searched for defaults
#  WIRE_ROOT_DIR: Base directory where all wire components are found
#
# Once done this will define
#  WIRE_FOUND - System has wire
#  WIRE_INCLUDE_DIRS - The wire cpp header include directories
#  WIRE_IDL_DIRECTORIES - The wire idl include directories
#  WIRE_LIBRARIES - The libraries needed to use wire
#  WIRE2CPP - Path to wire2cpp executable
#  WIRE2CPP_CMAKE - Path to wire2cpp.cmake script.

if(NOT WIRE_FOUND)

set(WIRE_ROOT_DIR "" CACHE PATH "Folder containing wire")

find_path(WIRE_INCLUDE_DIR "wire/version.hpp"
    PATHS ${WIRE_ROOT_DIR}
    PATH_SUFFIXES include
    NO_DEFAULT_PATH)
find_path(WIRE_INCLUDE_DIR "wire/version.hpp")

find_path(WIRE_IDL_DIR "wire/sugar.wire"
    PATHS ${WIRE_ROOT_DIR}
    PATH_SUFFIXES etc/share/wire/idl
    NO_DEFAULT_PATH)
find_path(WIRE_IDL_DIR "wire/sugar.wire"
    PATH_SUFFIXES etc/share/wire/idl)

find_library(WIRE_LIBRARY NAMES "wire"
  PATHS ${WIRE_ROOT_DIR}
  PATH_SUFFIXES lib lib64
  NO_DEFAULT_PATH)
find_library(WIRE_LIBRARY NAMES "wire")

find_program(WIRE2CPP NAMES "wire2cpp"
    PATHS ${WIRE_ROOT_DIR}
    PATH_SUFFIXES bin
    NO_DEFAULT_PATH)
find_program(WIRE2CPP NAMES "wire2cpp")

find_file(WIRE2CPP_CMAKE "wire2cpp.cmake"
    PATHS ${WIRE_ROOT_DIR}
    PATH_SUFFIXES etc/share/wire/cmake
    NO_DEFAULT_PATH)
find_file(WIRE2CPP_CMAKE "wire2cpp.cmake"
    PATH_SUFFIXES etc/share/wire/cmake)

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set benchmark_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(WIRE FOUND_VAR WIRE_FOUND
  REQUIRED_VARS WIRE_LIBRARY WIRE_INCLUDE_DIR WIRE_IDL_DIR WIRE2CPP WIRE2CPP_CMAKE)

if(WIRE_FOUND)
  set(WIRE_LIBRARIES ${WIRE_LIBRARY})
  set(WIRE_INCLUDE_DIRS ${WIRE_INCLUDE_DIR})
  set(WIRE_IDL_DIRECTORIES ${WIRE_IDL_DIR})
endif()

mark_as_advanced(WIRE_INCLUDE_DIR WIRE_IDL_DIR WIRE_LIBRARY)

endif()

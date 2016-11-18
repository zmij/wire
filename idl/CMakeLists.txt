#    /wire/idl/CMakeLists.txt
#
#    @author zmij
#    @date Nov 18, 2016

cmake_minimum_required(VERSION 2.6)

install(
    DIRECTORY wire
    DESTINATION etc/share/${PROJECT_NAME}/idl
    FILES_MATCHING PATTERN *.wire
)

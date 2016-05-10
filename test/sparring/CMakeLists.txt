#    /wire/test/sparring/CMakeLists.txt
#
#    @author zmij
#    @date May 10, 2016

cmake_minimum_required(VERSION 2.6)

set(sparring_SRCS sparring_test.cpp options.cpp)
add_library(sparring-test ${sparring_SRCS})
target_link_libraries(sparring-test
    ${GTEST_LIBRARIES}
    ${Boost_PROGRAM_OPTIONS_LIBRARIES}
    ${Boost_IOSTREAMS_LIBRARIES}
    ${Boost_SYSTEM_LIBRARIES}
    ${OPENSSL_LIBRARIES}
)

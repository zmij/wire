# CMakeLists.txt
# Created on: 25 февр. 2016 г.
#     Author: sergey.fedorov

cmake_minimum_required(VERSION 2.6)

set(
    wire_ast_SRCS
    ast.cpp
)

add_library(wire_ast SHARED ${wire_ast_SRCS})
cmake_minimum_required(VERSION 2.6)
project(volpack)

set(VOLPACK_LIBRARIES volpack PARENT_SCOPE)

file(GLOB CSRC "*.c")

add_library(volpack ${CSRC})

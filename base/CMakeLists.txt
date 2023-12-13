cmake_minimum_required(VERSION 3.21)

project(stuff)

include_directories(include)
add_subdirectory(src)
add_library(base STATIC ${LIB_SRCS})

cmake_minimum_required(VERSION 3.21)

project(stuff)

set(CMAKE_CXX_FLAGS "-std=gnu++17 -g")

add_executable(main url.cpp)

include(../stuff.cmake)
stuff_init(main)

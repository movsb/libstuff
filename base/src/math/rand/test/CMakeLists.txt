cmake_minimum_required(VERSION 3.21)

project(test CXX)
add_compile_options(-std=c++20)

add_subdirectory(.. rand)
link_libraries(rand)
include_directories(..)

add_executable(main main.cpp)

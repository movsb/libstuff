cmake_minimum_required(VERSION 3.21)

project(stuff)

set(CMAKE_CXX_FLAGS -std=gnu++17)

add_executable(main main.cpp)

add_subdirectory(base)

target_link_libraries(main PRIVATE base)

cmake_minimum_required(VERSION 3.21)

project(tests)

set(CMAKE_CXX_FLAGS "-std=gnu++17 -g")

add_executable(main main.cpp)

include(../_ports/cmake/stuff/CMakeLists.txt)
stuff_init_target(main)

cmake_minimum_required(VERSION 3.21)

project(stuff)

set(CMAKE_CXX_FLAGS "-std=gnu++17 -g")

add_executable(main main.cpp)

add_subdirectory($ENV{STUFF_PATH}/base)
target_include_directories(main PRIVATE $ENV{STUFF_PATH}/base/include)
target_link_libraries(main PRIVATE base)

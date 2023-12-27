#cmake_minimum_required(VERSION 3.21)
#
#project(test CXX)
#add_compile_options(-std=c++20)
#
#add_subdirectory(.. ssd1306)
#link_libraries(ssd1306)
#include_directories(..)
#
#
#include($ENV{IDF_PATH}/tools/cmake/project.cmake)
#
#add_executable(main main.cpp)
#
# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html
# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(tests)

add_subdirectory(.. ssd1306)
#  link_directories(build/ssd1306)
#  link_libraries(ssd1306 build/ssd1306)
#  target_link_libraries(esp32-c3 ssd1306)
# target_link_libraries(${CMAKE_PROJECT_NAME} ssd1306)

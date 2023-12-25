cmake_minimum_required(VERSION 3.21)

project(stuff)

add_subdirectory(net)
add_subdirectory(base)
add_library(esp8266 STATIC ${LIB_SRCS})
target_include_directories(esp8266 PRIVATE ../../base/include)
target_include_directories(esp8266 PRIVATE ../../net/include)

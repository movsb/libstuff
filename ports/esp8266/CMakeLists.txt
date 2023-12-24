cmake_minimum_required(VERSION 3.21)

project(stuff)

include_directories(include ../../net/include)
add_subdirectory(net)
add_library(esp8266 STATIC ${LIB_SRCS})

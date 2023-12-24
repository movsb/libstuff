cmake_minimum_required(VERSION 3.21)

project(stuff)

add_subdirectory(net)
add_library(esp8266 STATIC ${LIB_SRCS})

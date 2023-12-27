cmake_minimum_required(VERSION 3.21)

project(stuff)

add_library(target STATIC esp32.cpp)

file(GLOB INC ../esp32)
set(STUFF_INCLUDES ${STUFF_INCLUDES} ${INC} PARENT_SCOPE)

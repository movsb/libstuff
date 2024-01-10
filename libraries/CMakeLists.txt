cmake_minimum_required(VERSION 3.21)

project(stuff)

add_subdirectory(base)
add_subdirectory(drivers)
add_subdirectory(net)
add_subdirectory(modules)

add_library(stuff STATIC ${LIB_SRCS} ${STUFF_SRCS})
target_include_directories(stuff PRIVATE _include)

file(GLOB INC _include)
set(STUFF_INCLUDES ${STUFF_INCLUDES} ${INC} PARENT_SCOPE)

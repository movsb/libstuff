file(GLOB SRC *.cpp)
add_library(time ${SRC})
target_include_directories(time PRIVATE ../include)

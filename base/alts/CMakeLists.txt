file(GLOB SRC *.c *.cpp)
add_library(alts ${SRC})
target_include_directories(alts PRIVATE ../include)

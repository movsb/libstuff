# cmake_minimum_required(VERSION 3.21)

# project(stuff)

# add_subdirectory(app)

# add_library(target STATIC esp32.cpp ${LIB_SRCS})

# file(GLOB INC ../esp32)
# set(STUFF_INCLUDES ${STUFF_INCLUDES} ${INC} PARENT_SCOPE)

file(GLOB _SRCS *.cpp)
idf_component_register(SRCS ${_SRCS} INCLUDE_DIRS . REQUIRES app_update esp_https_ota)

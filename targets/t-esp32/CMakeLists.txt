# cmake_minimum_required(VERSION 3.21)

# project(stuff)

# add_subdirectory(app)

# add_library(target STATIC esp32.cpp ${LIB_SRCS})

# file(GLOB INC ../esp32)
# set(STUFF_INCLUDES ${STUFF_INCLUDES} ${INC} PARENT_SCOPE)

idf_component_register(
	SRC_DIRS .
	INCLUDE_DIRS .
	REQUIRES app_update esp_https_ota esp_http_server
)

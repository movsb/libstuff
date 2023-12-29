#cmake_minimum_required(VERSION 3.21)
#
#project(stuff)
#
# add_subdirectory(base)
# add_subdirectory(net)
# add_subdirectory(drivers)
#add_library(port STATIC ${LIB_SRCS})
#target_include_directories(port PRIVATE include)
#target_include_directories(port PRIVATE ../../libraries/_include)
#
#file(GLOB INC include)
#set(STUFF_INCLUDES ${STUFF_INCLUDES} ${INC} PARENT_SCOPE)
# file(GLOB BASE_SRCS base/*.cpp)
# file(GLOB DRIVERS_GPIO_SRCS drivers/gpio/*.cpp)
# file(GLOB NET_WIFI_SRCS net/wifi/*.cpp)
# file(GLOB NET_HTTP_SRCS net/http/*.cpp)
# set(LIB_SRCS ${BASE_SRCS} ${DRIVERS_GPIO_SRCS} ${NET_WIFI_SRCS} ${NET_HTTP_SRCS})
# message("ports/esp32:" ${LIB_SRCS})
idf_component_register(
	SRC_DIRS base drivers/gpio net/wifi
	INCLUDE_DIRS include ../../libraries/_include
	REQUIRES esp_http_server
)

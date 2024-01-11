stuff_add_sources(
	${CMAKE_CURRENT_LIST_DIR}
	wifi.cpp
)

if(ESP_PLATFORM)
	stuff_add_sources(
		${CMAKE_CURRENT_LIST_DIR}
		wifi_esp32.cpp
	)
endif()

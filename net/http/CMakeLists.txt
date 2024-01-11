stuff_add_sources(
	${CMAKE_CURRENT_LIST_DIR}
	status.cpp
)

if(ESP_PLATFORM)
	stuff_add_sources(
		${CMAKE_CURRENT_LIST_DIR}
		http_esp32.cpp
	)
endif()

stuff_add_sources(
	${CMAKE_CURRENT_LIST_DIR}
	nvs.cpp
	value.cpp
)

if(ESP_PLATFORM)
	stuff_add_sources(
		${CMAKE_CURRENT_LIST_DIR}
	)
endif()

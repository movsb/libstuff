stuff_add_sources(
	${CMAKE_CURRENT_LIST_DIR}
)

if(ESP_PLATFORM)
	stuff_add_sources(
		${CMAKE_CURRENT_LIST_DIR}
		homekit.cpp
	)
endif()

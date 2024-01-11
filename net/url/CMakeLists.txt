stuff_add_sources(
	${CMAKE_CURRENT_LIST_DIR}
	url.cpp
)

if(ESP_PLATFORM)
	stuff_add_sources(
		${CMAKE_CURRENT_LIST_DIR}
	)
endif()

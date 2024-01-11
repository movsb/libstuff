stuff_add_sources(
	${CMAKE_CURRENT_LIST_DIR}
	http.cpp
	status.cpp
)

if(ESP_PLATFORM)
	stuff_add_sources(
		${CMAKE_CURRENT_LIST_DIR}
	)
endif()

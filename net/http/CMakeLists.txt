stuff_add_sources(
	${CMAKE_CURRENT_LIST_DIR}
	status.cpp
	http.cpp
)

if(ESP_PLATFORM)
	stuff_add_sources(
		${CMAKE_CURRENT_LIST_DIR}
	)
endif()

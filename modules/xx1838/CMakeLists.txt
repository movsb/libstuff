stuff_add_sources(
	${CMAKE_CURRENT_LIST_DIR}
	decoder.cpp
	keycode.cpp
)

if(ESP_PLATFORM)
	stuff_add_sources(
		${CMAKE_CURRENT_LIST_DIR}
	)
endif()

stuff_add_sources(
	${CMAKE_CURRENT_LIST_DIR}
	shell.cpp
)

if(ESP_PLATFORM)
	stuff_add_sources(
		${CMAKE_CURRENT_LIST_DIR}
	)
	stuff_add_embed_texts(
		${CMAKE_CURRENT_LIST_DIR}
		shell.html
	)
endif()

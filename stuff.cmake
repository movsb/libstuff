function(stuff_init target cmakeTarget)
	message("TEST:" ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/libraries)
	add_subdirectory(${CMAKE_CURRENT_FUNCTION_LIST_DIR}/libraries stuff)
	add_subdirectory(${CMAKE_CURRENT_FUNCTION_LIST_DIR}/ports/${target} port)
	add_subdirectory(${CMAKE_CURRENT_FUNCTION_LIST_DIR}/targets/${target} target)
	target_link_libraries(${cmakeTarget} stuff port target)
	message(${cmakeTarget} "STUFF_INCLUDES:" ${STUFF_INCLUDES})
	target_include_directories(${cmakeTarget} PRIVATE ${STUFF_INCLUDES})
endfunction()
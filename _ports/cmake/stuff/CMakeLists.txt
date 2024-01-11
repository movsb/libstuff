cmake_minimum_required(VERSION 3.21)

project(stuff)

set(__STUFF_SRCS)
set(__STUFF_EMBED_TEXTS)

function(stuff_add_sources dir)
	set(__SRCS)
	foreach(src ${ARGN})
		set(__SRCS ${__SRCS} ${dir}/${src})
	endforeach()
	set(__STUFF_SRCS ${__STUFF_SRCS} ${__SRCS} PARENT_SCOPE)
endfunction()

function(stuff_add_embed_texts dir)
	set(__FILES)
	foreach(file ${ARGN})
		set(__FILES ${__FILES} ${dir}/${file})
	endforeach()
	set(__STUFF_EMBED_TEXTS ${__STUFF_EMBED_TEXTS} ${__FILES} PARENT_SCOPE)
endfunction()

set(__sb ${CMAKE_CURRENT_LIST_DIR}/../../..)

set(subDirs
	base/alts
	base/time
	drivers/gpio
	modules/xx1838
	net/http
	net/url
	net/wifi
	os/timer
	protos/homekit
	storage/nvs
	tools/shell
)

foreach(sub ${subDirs})
	include(${__sb}/${sub}/CMakeLists.txt)
endforeach()

add_library(stuff STATIC ${__STUFF_SRCS})
target_include_directories(stuff PRIVATE ${__sb}/_include)

function(stuff_set_port_includes)
	foreach(inc ${ARGN})
		target_include_directories(stuff PRIVATE ${inc})
	endforeach()
endfunction()

function(stuff_init_target cmakeTarget)
	target_link_libraries(${cmakeTarget} stuff)
	target_include_directories(${cmakeTarget} PRIVATE ${__sb}/_include)
endfunction()

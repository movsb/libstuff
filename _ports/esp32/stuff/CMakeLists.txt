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
	base/log
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

message("STUFF_SRCS:" ${__STUFF_SRCS})
message("STUFF_INC:" ${__sb}/_include)

idf_component_register(
	SRCS ${__STUFF_SRCS}
	INCLUDE_DIRS ${__sb}/_include
	REQUIRES app_update esp_https_ota esp_http_server freertos stuff
		nvs_flash
		esp_hap_apple_profiles
		esp_hap_core
		esp_http_server
	EMBED_TXTFILES ${__STUFF_EMBED_TEXTS}
)

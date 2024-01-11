set(EXTRA_COMPONENT_DIRS
	$ENV{IDF_PATH}/../esp-homekit-sdk/components
	$ENV{IDF_PATH}/../esp-homekit-sdk/components/homekit
)

message("HOMEKIT_DIR:" $ENV{IDF_PATH}/../esp-homekit-sdk/)

set(__STUFF_SRCS)

function(stuff_add_sources dir)
	foreach(src ${ARGN})
		set(__STUFF_SRCS ${__STUFF_SRCS} ${dir}/${src})
	endforeach()
endfunction()

if(ESP_PLATFORM)
	add_compile_definitions(_D__STUFF_ESP32__)
endif()

add_subdirectory(../../../base)
add_subdirectory(../../../drivers)
add_subdirectory(../../../modules)
add_subdirectory(../../../net)
add_subdirectory(../../../os)
add_subdirectory(../../../protos)
add_subdirectory(../../../storage)
add_subdirectory(../../../tools)

idf_component_register(
	SRCS ${__STUFF_SRCS}
	INCLUDE_DIRS ../../../
	REQUIRES app_update esp_https_ota esp_http_server freertos stuff
		nvs_flash
		esp_hap_apple_profiles
		esp_hap_core
		esp_http_server
	EMBED_TXTFILES
)

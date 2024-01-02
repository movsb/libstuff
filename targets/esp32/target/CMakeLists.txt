set(EXTRA_COMPONENT_DIRS
	$ENV{IDF_PATH}/../esp-homekit-sdk/components
	$ENV{IDF_PATH}/../esp-homekit-sdk/components/homekit
)

message("HOMEKIT_DIR:" $ENV{IDF_PATH}/../esp-homekit-sdk/)

idf_component_register(
	SRC_DIRS
		..
		../freertos
		../net/http
		../storage/nvs
	INCLUDE_DIRS ..
	REQUIRES app_update esp_https_ota esp_http_server freertos stuff
		nvs_flash
		esp_hap_apple_profiles
		esp_hap_core
)

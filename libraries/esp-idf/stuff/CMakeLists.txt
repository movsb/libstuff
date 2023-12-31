idf_component_register(
	SRC_DIRS
		../../base/alts
		../../base/math
		../../base/sync
		../../base/time
		../../drivers/gpio
		../../net/http
		../../net/wifi
		../../net/url
	INCLUDE_DIRS
		../include
		../../../libraries/_include
	REQUIRES esp_http_server port
)

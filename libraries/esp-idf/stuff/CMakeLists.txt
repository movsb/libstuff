idf_component_register(
	SRC_DIRS
		../../base/alts
		../../base/math/rand
		../../base/time
		../../drivers/gpio
		../../net/wifi
		../../net/url
	INCLUDE_DIRS
		../include
		../../../libraries/_include
	REQUIRES esp_http_server port
)

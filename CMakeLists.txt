idf_component_register(
	SRCS "stub.cpp" "net.cpp" "wifi.cpp" "time.cpp" "os.cpp" "io.cpp"
	INCLUDE_DIRS "include"
	PRIV_REQUIRES esp_netif esp_wifi esp_timer vfs driver
)

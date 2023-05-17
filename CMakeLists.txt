idf_component_register(
	SRCS "src/stub.cpp" "src/net.cpp" "src/wifi.cpp" "src/time.cpp" "src/os.cpp" "src/io.cpp" "src/http.cpp"
	INCLUDE_DIRS "include"
	PRIV_REQUIRES esp_netif esp_wifi esp_timer vfs driver esp_http_client
)

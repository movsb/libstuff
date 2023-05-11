#include "ebp/net.hpp"

#include <cstring>
#include <strings.h>

#include <esp_netif.h>
#include <esp_netif_defaults.h>
#include <esp_wifi.h>

#include "ebp/wifi.hpp"

static const char *tag = "wifi";
static const char *desc = "ebp_sta";

namespace ebp {
namespace wifi {

esp_err_t Station::start()
{
	wifi_init_config_t initConfig = WIFI_INIT_CONFIG_DEFAULT();
	if (auto err = ::esp_wifi_init(&initConfig); err != ESP_OK) {
		return err;
	}

	esp_netif_inherent_config_t netifConfig = ESP_NETIF_INHERENT_DEFAULT_WIFI_STA();
	netifConfig.if_desc = desc;
	netifConfig.route_prio = 128;
	_netif = ::esp_netif_create_wifi(WIFI_IF_STA, &netifConfig);
	esp_wifi_set_default_wifi_sta_handlers();
	
	if (auto err = esp_wifi_set_storage(WIFI_STORAGE_RAM); err != ESP_OK) {
		return err;
	}
	if (auto err = esp_wifi_set_mode(WIFI_MODE_STA); err != ESP_OK) {
		return err;
	}
	if (auto err = esp_wifi_start(); err != ESP_OK) {
		return err;
	}
	
	return ESP_OK;
}

void Station::stop()
{
	switch (auto err = esp_wifi_stop(); err) {
	case ESP_ERR_WIFI_NOT_INIT:
		ESP_LOGW(tag, "stop while wifi is not initialized");
		return;
	case ESP_OK:
		break;
	default:
		ESP_LOGE(tag, "unknown error: %x", err);
	}
	
	if (auto err = esp_wifi_deinit(); err != ESP_OK) {
		ESP_LOGE(tag, "deinit failed: %x", err);
		return;
	}
	
	esp_wifi_clear_default_wifi_driver_and_handlers(_netif);
	esp_netif_destroy(_netif);
	_netif = nullptr;
	
	ESP_LOGI(tag, "stopped");
}

esp_err_t (Station::connect)(const char *ssid, const char *password)
{
	_semGotIPv4 = xSemaphoreCreateBinary();
	if (_semGotIPv4 == nullptr) {
		ESP_LOGE(tag, "not enough memory to create semaphore");
		return ESP_ERR_NO_MEM;
	}

	wifi_config_t wifiConfig;
	bzero(&wifiConfig, sizeof(wifiConfig));
	auto &sta = wifiConfig.sta;
	
	if (ssid == nullptr || std::strlen(ssid) >= std::size(sta.ssid)) {
		return ESP_ERR_INVALID_ARG;
	}
	if (password == nullptr || std::strlen(password) >= std::size(sta.password)) {
		return ESP_ERR_INVALID_ARG;
	}

	std::strcpy(reinterpret_cast<char*>(&sta.ssid), ssid);
	std::strcpy(reinterpret_cast<char*>(&sta.password), password);
	
	sta.scan_method = WIFI_FAST_SCAN;
	sta.sort_method = WIFI_CONNECT_AP_BY_SIGNAL;
	sta.threshold.rssi = -127;
	sta.threshold.authmode = WIFI_AUTH_OPEN;
	
	if (auto err = esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_CONNECTED, _onConnected, this); err != ESP_OK) {
		return err;
	}
	if (auto err = esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, _onDisconnected, this); err != ESP_OK) {
		return err;
	}
	if (auto err = esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, _onGotIPv4, this); err != ESP_OK) {
		return err;
	}
	
	ESP_LOGI(tag, "Connecting to %s...", ssid);
	_retryCount = 0;
	if (auto err = esp_wifi_set_config(WIFI_IF_STA, &wifiConfig); err != ESP_OK) {
		ESP_LOGE(tag, "Wifi configuration failed: %x", err);
		return err;
	}
	if (auto err = esp_wifi_connect(); err != ESP_OK) {
		ESP_LOGE(tag, "Wifi connection failed: %x", err);
		return err;
	}
	
	// 等到拿到 IP 地址。
	ESP_LOGI(tag, "Waiting for IP address");
	xSemaphoreTake(_semGotIPv4, portMAX_DELAY);
	
	if (_maxRetryCount > 0 && _retryCount > _maxRetryCount) {
		ESP_LOGE(tag, "reached max retry count limit: %d", _maxRetryCount);
		return ESP_FAIL;
	}
	
	return ESP_OK;
}

esp_err_t Station::disconnect()
{
	esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_STA_CONNECTED, _onConnected);
	esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, _onDisconnected);
	esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, _onGotIPv4);
	
	vSemaphoreDelete(_semGotIPv4);
	_semGotIPv4 = nullptr;

	return esp_wifi_disconnect();
}

void Station::onConnected(esp_event_base_t eventBase, int32_t eventID, void *eventData)
{
	ESP_LOGI(tag, "connected");
}

void Station::onDisconnected(esp_event_base_t eventBase, int32_t eventID, void *eventData)
{
	ESP_LOGI(tag, "disconnected");

	_retryCount++;
	if (_maxRetryCount > 0 && _retryCount > _maxRetryCount) {
		ESP_LOGE(tag, "reached max retry count, won't retry again");
		xSemaphoreGive(_semGotIPv4);
		return;
	}
	
	if (auto err = esp_wifi_connect(); err != ESP_OK) {
		ESP_LOGE(tag, "error reconnecting: %x", err);
		return;
	}

	ESP_LOGI(tag, "reconnecting");
}

void Station::onGotIPv4(esp_event_base_t eventBase, int32_t eventID, ip_event_got_ip_t *event)
{
	_retryCount = 0;

	// 不是我们的接口
	if (std::strncmp(desc, esp_netif_get_desc(event->esp_netif), std::strlen(desc)) != 0) {
		return;
	}

	ESP_LOGI(tag, "got IPv4, interface: %s, address: " IPSTR ", gateway: " IPSTR ", netmask: " IPSTR,
		esp_netif_get_desc(event->esp_netif), IP2STR(&event->ip_info.ip),
		IP2STR(&event->ip_info.gw), IP2STR(&event->ip_info.netmask)
	);
	
	xSemaphoreGive(_semGotIPv4);
}
} // namespace wifi
} // namespace ebp

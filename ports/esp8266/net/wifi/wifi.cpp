#include "sdkconfig.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include <esp_log.h>
#include "esp_event_loop.h"
#include "tcpip_adapter.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "lwip/err.h"
#include "lwip/sys.h"

#include <algorithm>
#include <cstring>

#include <stuff/net/wifi.hpp>

#define GOT_IPV4_BIT BIT(0)
#define CONNECTED_BITS (GOT_IPV4_BIT)

static const char *TAG = "example_connect";

static void _init_wifi() {
	static bool _initialized = false;
	if (_initialized) { return; }
	tcpip_adapter_init();
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
	_initialized = true;
}

/**
 * @todo 只能创建一个实例。
*/
class Station : public stuff::net::wifi::_Station {
public:
	Station()
	{
		_init();
		_event_group = xEventGroupCreate();
	}
	virtual ~Station() {
		// _uninit();
		vEventGroupDelete(_event_group);
	}
public:
	virtual int connect(const char *ssid, const char *password) override {
		wifi_config_t cfg;
		std::memset(&cfg, 0, sizeof(cfg));
		int m = strlen(ssid), n = password ? strlen(password) : 0;
		if (m > sizeof(cfg.sta.ssid) - 1) {
			ESP_LOGE(TAG, "ssid [%s] too long", ssid);
			return -1;
		}
		if (n > sizeof(cfg.sta.password) - 1) {
			ESP_LOGE(TAG, "password [%s] too long", password);
			return -1;
		}
		std::memcpy(cfg.sta.ssid, ssid, m+1);
		std::memcpy(cfg.sta.password, password, n ? n + 1 : 0);
		
		ESP_LOGI(TAG, "Connecting to [%s]", ssid);

		wifi_mode_t mode;
		ESP_ERROR_CHECK(esp_wifi_get_mode(&mode));
		switch (mode) {
		case WIFI_MODE_AP:  mode = WIFI_MODE_APSTA; break;
		default:            mode = WIFI_MODE_STA;   break;
		}

		ESP_ERROR_CHECK(esp_wifi_set_mode(mode));
		ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &cfg));
		
		ESP_ERROR_CHECK(esp_wifi_start());
		ESP_ERROR_CHECK(esp_wifi_connect());

		xEventGroupWaitBits(_event_group, CONNECTED_BITS, true, true, portMAX_DELAY);
		
		return 0;
	}
	virtual void disconnect() override {
		esp_wifi_disconnect();
	}

protected:
	void _init() {
		_init_wifi();
		ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_CONNECTED, &_onConnected, this));
		ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &_onDisconnected, this));
		ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &_onGotIPv4, this));
	}
	/*
	void _uninit(void) {
		esp_err_t err = esp_wifi_stop();
		if (err == ESP_ERR_WIFI_NOT_INIT) {
			return;
		}
		ESP_ERROR_CHECK(err);

		ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_STA_CONNECTED, &_onConnected));
		ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &_onDisconnected));
		ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &_onGotIPv4));

		ESP_ERROR_CHECK(esp_wifi_deinit());
	}
	*/
private:
	static void _onConnected(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
		auto that = reinterpret_cast<Station*>(arg);
		auto &event = *reinterpret_cast<system_event_sta_connected_t*>(event_data);
		return that->onConnected(event);
	}
	void onConnected(system_event_sta_connected_t &event) {
		ESP_LOGI(TAG, "WiFi connected to [%*s]", event.ssid_len, event.ssid);
	}
	static void _onDisconnected(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
		auto that = reinterpret_cast<Station*>(arg);
		auto &event = *reinterpret_cast<system_event_sta_disconnected_t*>(event_data);
		return that->onDisconnected(event);
	}
	void onDisconnected(system_event_sta_disconnected_t &event) {
		ESP_LOGI(TAG, "WiFi disconnected from [%*s], trying to reconnect...", event.ssid_len, reinterpret_cast<const char*>(event.ssid));
		
		if (event.reason == WIFI_REASON_BASIC_RATE_NOT_SUPPORT) {
			// Switch to 802.11 bgn mode
			esp_wifi_set_protocol(ESP_IF_WIFI_STA, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N);
		}
		
		ESP_ERROR_CHECK(esp_wifi_connect());
	}
	static void _onGotIPv4(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
		auto that = reinterpret_cast<Station*>(arg);
		auto &event = *reinterpret_cast<ip_event_got_ip_t*>(event_data);
		return that->onGotIPv4(event);
	}
	void onGotIPv4(ip_event_got_ip_t &event) {
		std::memcpy(&_ipv4, &event.ip_info.ip, sizeof(_ipv4));
		xEventGroupSetBits(_event_group, GOT_IPV4_BIT);
	}

private:
	ip4_addr_t _ipv4;
	EventGroupHandle_t _event_group;
};


namespace stuff {
namespace net {
namespace wifi {

_Station* __new_station() {
	return new ::Station();
}

void __delete_station(_Station* station) {
	delete reinterpret_cast<::Station*>(station);
}

}
}
}

/**
 * @todo 只能创建一个实例。
*/
class AccessPoint : public stuff::net::wifi::_AccessPoint {
public:
	AccessPoint()
	{
		_init();
	}
	virtual ~AccessPoint() {
		// _uninit();
	}
public:
	virtual int start(const char *ssid, const char *password) override {
		wifi_config_t cfg;
		std::memset(&cfg, 0, sizeof(cfg));
		int m = strlen(ssid), n = password ? strlen(password) : 0;
		if (m <= 0) {
			ESP_LOGE(TAG, "Invalid ssid");
			return -1;
		}
		if (m > sizeof(cfg.sta.ssid) - 1) {
			ESP_LOGE(TAG, "ssid [%s] too long", ssid);
			return -1;
		}
		if (n > sizeof(cfg.sta.password) - 1) {
			ESP_LOGE(TAG, "password [%s] too long", password);
			return -1;
		}

		auto &ap = cfg.ap;
		// WIFI_AUTH_WPA3_PSK 会崩溃，暂时不使用
		ap.authmode = n > 0 ? WIFI_AUTH_WPA2_PSK : WIFI_AUTH_OPEN;
		ap.max_connection = 1;
		ap.ssid_hidden = 0;

		std::memcpy(ap.ssid, ssid, m+1);
		std::memcpy(ap.password, password, n ? n + 1 : 0);
		
		wifi_mode_t mode;
		ESP_ERROR_CHECK(esp_wifi_get_mode(&mode));
		switch (mode) {
		case WIFI_MODE_STA: mode = WIFI_MODE_APSTA; break;
		default:            mode = WIFI_MODE_AP;   break;
		}

		ESP_ERROR_CHECK(esp_wifi_set_mode(mode));
		ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &cfg));
		
		ESP_ERROR_CHECK(esp_wifi_start());

		return 0;
	}

protected:
	void _init() {
		_init_wifi();
		ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &_eventHandler, this));
	}
	/*
	void _uninit(void) {
		esp_err_t err = esp_wifi_stop();
		if (err == ESP_ERR_WIFI_NOT_INIT) {
			return;
		}
		ESP_ERROR_CHECK(err);

		ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &_eventHandler));

		ESP_ERROR_CHECK(esp_wifi_deinit());
	}
	*/
private:
	static void _eventHandler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
		auto that = reinterpret_cast<AccessPoint*>(arg);
		return that->eventHandler(event_base, event_id, event_data);
	}
	void eventHandler(esp_event_base_t event_base,int32_t event_id, void *event_data) {
	}

private:
};


namespace stuff {
namespace net {
namespace wifi {

_AccessPoint* __new_access_point() {
	return new ::AccessPoint();
}

void __delete_access_point(_AccessPoint* ap) {
	delete reinterpret_cast<::AccessPoint*>(ap);
}

}
}
}

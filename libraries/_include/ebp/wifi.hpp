#pragma once

#include <string>
#include <functional>
#include <sys/time.h>

#include "base.hpp"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_netif_types.h>

namespace ebp {
namespace wifi {

// Station 即常用的主动连接方设备。例如手机、平板。
// 对应的是 AP 模式，即无线路由器（WiFi）。
class Station {
public:
	Station()
		: _maxRetryCount(0)
		, _netif(nullptr)
		{
		}

public:
	esp_err_t start();
	void stop();
	esp_err_t (connect)(const char *ssid, const char *password);
	esp_err_t (disconnect)();
	
private:
	static void _onConnected(void *arg, esp_event_base_t eventBase, int32_t eventID, void *eventData) {
		auto that = reinterpret_cast<Station*>(arg);
		return that->onConnected(eventBase, eventID, eventData);
	}
	void onConnected(esp_event_base_t eventBase, int32_t eventID, void *eventData);
	static void _onDisconnected(void *arg, esp_event_base_t eventBase, int32_t eventID, void *eventData) {
		auto that = reinterpret_cast<Station*>(arg);
		return that->onDisconnected(eventBase, eventID, eventData);
	}
	void onDisconnected(esp_event_base_t eventBase, int32_t eventID, void *eventData);
	static void _onGotIPv4(void *arg, esp_event_base_t eventBase, int32_t eventID, void *eventData) {
		auto that = reinterpret_cast<Station*>(arg);
		return that->onGotIPv4(eventBase, eventID, reinterpret_cast<ip_event_got_ip_t*>(eventData));
	}
	void onGotIPv4(esp_event_base_t eventBase, int32_t eventID, ip_event_got_ip_t *event);
	
private:
	// 最大重试次数，0 的时候不限制
	uint8_t _maxRetryCount;
	// 开始连接或者连接成功后置零，每次失败加一。
	uint8_t _retryCount;
	
private:
	esp_netif_t *_netif;
	SemaphoreHandle_t _semGotIPv4;
};

} // namespace wifi
} // namespace ebp

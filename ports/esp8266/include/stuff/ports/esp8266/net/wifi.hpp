#pragma once

#include <stuff/net/wifi.hpp>

namespace stuff {
namespace ports {
namespace esp8266 {
namespace net {
namespace wifi {

// Station 即常用的主动连接方设备。例如手机、平板。
// 对应的是 AP 模式，即无线路由器（WiFi）。
class Station : public stuff::net::wifi::Station {
public:
	virtual int connect(const char *ssid, const char *password);
};


} // namespace wifi
} // namespace net
} // namespace esp8266
} // namespace ports
} // namespace stuff

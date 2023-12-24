#pragma once

namespace stuff {
namespace net {
namespace wifi {

// Station 即常用的主动连接方设备。例如手机、平板。
// 对应的是 AP 模式，即无线路由器（WiFi）。
class Station {
public:
	int connect(const char *ssid, const char *password);
};

} // namespace wifi
} // namespace net
} // namespace stuff

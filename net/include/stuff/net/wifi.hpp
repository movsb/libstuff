#pragma once

#include <cstdlib>

namespace stuff {
namespace net {
namespace wifi {

struct _Station {
	virtual int connect(const char *ssid, const char *password) = 0;
	virtual void disconnect() = 0;
};

extern _Station* __new_station();
extern void      __delete_station(_Station *station);

// Station 即常用的主动连接方设备。例如手机、平板。
// 对应的是 AP 模式，即无线路由器（WiFi）。
class Station {
public:
	Station()
	{
		_impl = __new_station();
		if (_impl == nullptr) {
			abort();
		}
	}
	virtual ~Station() {
		__delete_station(_impl);
	}
public:
	int connect(const char *ssid, const char *password) {
		return _impl->connect(ssid, password);
	}
protected:
	_Station *_impl;
};

} // namespace wifi
} // namespace net
} // namespace stuff

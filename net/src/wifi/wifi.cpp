#include <stuff/net/wifi.hpp>

extern "C" {

int _stuff_net_wifi_connect(const char *ssid, const char *password);

}

namespace stuff {
namespace net {
namespace wifi {

int Station::connect(const char *ssid, const char *password) {
	return _stuff_net_wifi_connect(ssid, password);
}

} // namespace wifi
} // namespace net
} // namespace stuff

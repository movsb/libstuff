#include "ebp/net.hpp"

#include <cstring>
#include <strings.h>
#include <charconv>

namespace ebp {
namespace net {

std::tuple<std::string, std::string, esp_err_t>
splitHostPort(const char *hostPort)
{
	const char *colon = std::strchr(hostPort, ':');
	if (!colon) {
		return {{}, {}, ESP_FAIL};
	}
	
	auto host = std::string(hostPort, colon);
	auto port = std::string(colon+1);
	
	return { host, port, ESP_OK };
}

std::tuple<std::unique_ptr<Conn>, esp_err_t>
Dialer::dial(const char *network, const char *address)
{
	auto [host, port, err1] = splitHostPort(address);
	if (err1 != ESP_OK) {
		return { nullptr, ESP_FAIL};
	}

	auto [ip, err2] = Resolver::lookupIP("tcp4", host.c_str());
	if (err2 != ESP_OK) {
		return { nullptr, err2 };
	}
	
	auto [nPort, ok] = alt::atoi<uint16_t>(port.c_str());
	if (!ok) {
		return { nullptr, ESP_FAIL};
	}

	int s = ::socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0) {
		return {nullptr, errno};
	}

	sockaddr_in sin;
	bzero(&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr = ip.toInAddr4();
	sin.sin_port = lwip_htons(nPort);
	
	if (::connect(s, reinterpret_cast<sockaddr*>(&sin), sizeof(sin)) !=0) {
		::lwip_close(s);
		return { nullptr, ESP_FAIL };
	}
	
	return { std::make_unique<TCPConn>(s), esp_err_t(ESP_OK) };
}

std::tuple<IP, esp_err_t> Resolver::lookupIP(const char* network, const char* host)
{
	addrinfo hints;
	bzero(&hints, sizeof(hints));
	hints.ai_family = PF_INET;
	hints.ai_socktype = SOCK_STREAM;
	
	addrinfo *res;
	
	printf("before getaddrinfo\n");
	int err = getaddrinfo(host, nullptr, &hints, &res);
	if (err != 0) {
		return {IP{}, esp_err_t(ESP_FAIL)};
	}

	in_addr *addr = &((sockaddr_in*)res->ai_addr)->sin_addr;
	ip4_addr_t addr4 = {addr->s_addr};
	uint8_t b1 = ip4_addr1_val(addr4),
		b2 = ip4_addr2_val(addr4),
		b3 = ip4_addr3_val(addr4),
		b4 = ip4_addr4_val(addr4);
	
	::freeaddrinfo(res);

	return {IP::v4(b1, b2, b3, b4), ESP_OK};
}

} // namespace net
} // namespace ebp

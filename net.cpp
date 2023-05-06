#include "ebp/net.hpp"

#include <lwip/netdb.h>

namespace ebp {
namespace net {

std::tuple<Conn*, esp_err_t>
dial(const std::string &network, const std::string &address)
{
	abort();
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

	return {IP::v4(b1, b2, b3, b4), ESP_OK};
}

} // namespace net
} // namespace ebp

#pragma once

#include "base.hpp"

#include <tuple>
#include <string>

namespace ebp {
namespace net {

struct Addr
{
	virtual std::string
	network() = 0;

	virtual std::string
	string() = 0;
};

struct Conn
{
	virtual std::tuple<int, esp_err_t>
	read(uint8_t *buf, int len) = 0;

	virtual std::tuple<int, esp_err_t>
	write(uint8_t *buf, int len) = 0;

	virtual esp_err_t
	close() = 0;
	
	virtual Addr
	localAddr() = 0;

	virtual Addr
	remoteAddr() = 0;
};

std::tuple<Conn*, esp_err_t>
dial(const char *network, const char *address);

struct IP
{
	bool _isV4;
	union {
		uint8_t _v4[4];
		uint8_t _v6[16];
	};
	
	static IP v4(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4) {
		IP ip;
		ip._isV4 = true;
		ip._v4[0] = b1;
		ip._v4[1] = b2;
		ip._v4[2] = b3;
		ip._v4[3] = b4;
		return ip;
	}
	
	std::string toString() const {
		char buf[128];
		if (_isV4) {
			snprintf(&buf[0], sizeof(buf)/sizeof(buf[0]), "%d.%d.%d.%d", _v4[0], _v4[1], _v4[2], _v4[3]);
			return buf;
		}
		// TODO:
		abort();
	}
};

class Resolver {
public:
	static std::tuple<IP, esp_err_t> lookupIP(const char* network, const char* host);
};

}
}

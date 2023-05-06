#pragma once

#include "base.hpp"

#include <tuple>
#include <string>
#include <memory>

#include <lwip/netdb.h>
#include <lwip/sockets.h>

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
	virtual std::tuple<int, esp_err_t> (read)(void *buf, int len) = 0;
	
	template<std::size_t N> std::tuple<int, esp_err_t> (read)(uint8_t (&buf)[N]) {
		return (read)(buf, N);
	}
	template<std::size_t N> std::tuple<int, esp_err_t> (read)(int8_t (&buf)[N]) {
		return (read)(buf, N);
	}
	
	virtual std::tuple<int, esp_err_t> (write)(const void *buf, int len) = 0;
	
	std::tuple<int, esp_err_t> (write)(const char *buf) {
		return (write)(buf, std::strlen(buf));
	}

	virtual esp_err_t close() = 0;
	
	virtual std::unique_ptr<Addr> localAddr() = 0;

	virtual std::unique_ptr<Addr> remoteAddr() = 0;
};

class TCPConn : public Conn
{
public:
	TCPConn(int fd) : _fd(fd) {}
	virtual ~TCPConn() { ::close(_fd); }

public:
	virtual std::tuple<int, esp_err_t>
	(read)(void *buf, int len) override
	{
		int r = ::lwip_read(_fd, buf, len);
		if (r == 0) {
			return {0, kEOF};
		}
		if (r < 0) {
			return {0, ESP_FAIL};
		}
		return {r, ESP_OK};
	}

	virtual std::tuple<int, esp_err_t>
	(write)(const void *buf, int len) override
	{
		int wrote = 0;
		while(wrote < len) {
			int r = ::lwip_write(_fd, reinterpret_cast<const char*>(buf)+wrote, len-wrote);
			if (r < 0) {
				return {0, ESP_FAIL};
			}
			if (r == 0) {
				abort();
			}
			wrote += r;
		}
		
		return {wrote, ESP_OK};
	}

	virtual esp_err_t
	close() override
	{
		return ::lwip_close(_fd);
	}
	
	virtual std::unique_ptr<Addr>
	localAddr() override
	{
		return nullptr;
	}

	virtual std::unique_ptr<Addr>
	remoteAddr() override
	{
		return nullptr;
	}

private:
	int     _fd;
};

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
	
	in_addr toInAddr4() const {
		if (!_isV4) {
			abort();
		}
		
		in_addr addr;
		addr.s_addr = PP_HTONL(LWIP_MAKEU32(_v4[0], _v4[1], _v4[2], _v4[3]));
		return addr;
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

std::tuple<std::string, std::string, esp_err_t>
splitHostPort(const char *hostPort);

class Resolver {
public:
	static std::tuple<IP, esp_err_t> lookupIP(const char* network, const char* host);
};

class Dialer {
public:
	static std::tuple<std::unique_ptr<Conn>, esp_err_t>
	dial(const char *network, const char *address);
};

}
}

#pragma once

#include <ebp/base.hpp>

#include <tuple>
#include <memory>
#include <string>

#include <string.h>
#include <stdlib.h>
#include <esp_event.h>
#include <esp_netif.h>
#include <esp_http_client.h>

namespace ebp {
namespace net {
namespace http {

using Method = esp_http_client_method_t;

class Client;

class Request {
public:
	Request(Method method, const std::string &url) {
		_method = method;
		_url = url;
	}

protected:
	Method          _method;
	std::string     _url;
	
	std::string     _proto;
	uint8_t         _protoMajor  :4;
	uint8_t         _protoMinor  :4;
	
	friend class Client;
};

class Response {
public:
	Response(Client &client) : _client(client) {}

	friend class Client;
protected:
	Client &_client;
};
	
} // namespace http
} // namespace net
} // namespace ebp

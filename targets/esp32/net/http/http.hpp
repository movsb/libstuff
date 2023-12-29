#pragma once

#include <tuple>
#include <memory>
#include <vector>
#include <functional>
#include <unordered_map>

#include <cstring>
#include <cstdlib>
#include <strings.h>
#include <stdint.h>

namespace stuff {
namespace net {
namespace http {

namespace internal {

} // namespace internal

/**
 * @brief HTTP 方法列表。
*/
struct Method {
	enum Value {
		Get,
		Post,
	};
};

/**
 * @brief Request 请求。
*/
class Request {
	friend class Server;
public:
	Request(Server* server, void *impl)
		: _server(server)
		, _impl(impl)
	{ }
private:
	Server *_server;
	void *_impl;
};

/**
 * @brief Response 响应。
*/
class Response {
	friend class Server;
public:
	Response(Server* server, void *impl)
		: _server(server)
		, _impl(impl)
	{ }
private:
	Server *_server;
	void *_impl;
};

typedef void (*HandlerFunc)(Request &request, Response &response);

/**
 * @brief HTTP 服务器实现类。
*/
class Server {
public:
	Server();
public:
	void handle(Method::Value method, const char *path, HandlerFunc handler);
public:
	std::tuple<Response, int> get(Request &request);
	std::tuple<Response, int> post(Request &request);

private:
	void _handle(HandlerFunc handler, Request &request);

protected:
	void *_impl;
};

} // namespace http
} // namespace net
} // namespace stuff

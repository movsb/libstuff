#pragma once

/**
 * @note 为后期移植考虑，本文件尽量不引入 ESP-IDF 的任何东西。
*/

#include <string>
#include <functional>
#include <cstring>
#include <cstdlib>
#include <strings.h>
#include <stdint.h>

#include "status.hpp"

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

class Server;

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
public:
	bool getHeader(const char* key, std::string *value);
	int read(void *buf, size_t len);
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
		, _statusSet(false)
		, _contentTypeSet(false)
		, _failed(false)
		, _isWritingChunkEnd(false)
	{ }
public:
	void setStatus(status::Code code);
	void setStatus(int code);
	/**
	 * @brief 设置头部 key-value 对。
	 * @note 如果是设置 Content-Type
	 * @note 内部在发送前会持有这2️⃣个指针。
	*/
	void setHeader(const char * const &key, const char * const &value);
	/**
	 * @brief 写数据到响应。
	 * 
	 * 如果没有 \p setStatus 或 设置 \p Content-Type ，会分别设置为：
	 *   - 200 OK
	 *   - Content-Type: text/plain; charset=utf-8
	 * 这和 ESP-IDF 的默认值不太一样（更标准？）。
	 * 
	 * 另一个不同之处在于：这个函数目前总是以 `Transfer-Encoding: chunked` 发送。
	 * 所以可以调用此函数为多次以构造响应体。也不必得像 ESP-IDF 那样要在最后以 len==0 调用一次。
	*/
	int write(const void *buf, size_t len);
	int write(const char *buf);
private:
	Server *_server;
	void *_impl;
	bool _statusSet;
	bool _contentTypeSet;
	/**
	 * \p HandlerFunc 在设计上返回值并没有实际意义，否则在处理函数内要处理较多的错误。
	 * 而错误一般产生于 write body 的时候，此时用户应该自己判断写错误。并且，后续如果增加了
	 * io.Writer 实现，基本可以不作错误判断了。ESP 要求 handler 返回错误码以关闭连接。
	*/
	bool _failed;
	// 只有在 Server::handle 结束的时候允许写 ChunkEnd，其它时候忽略 write 的空的调用。
	bool _isWritingChunkEnd;
};

typedef void HandlerFunc(Request &request, Response &response);


/**
 * @brief HTTP 服务器实现类。
*/
class Server {
public:
	Server();
	Server(uint16_t port);
	virtual ~Server();

public:
	void handle(Method::Value method, const char *path, std::function<HandlerFunc> handler);

private:
	bool _handle(std::function<HandlerFunc> h, Request &r, Response &w);

protected:
	friend class Request;
	friend class Response;
	friend class _CallServerHandle;

	void *_impl;
};

#define wrap(h) ([this](Request &r, Response &w){ h(r, w); })

} // namespace http
} // namespace net
} // namespace stuff

#include <cstdlib>

#include "http.hpp"

#include <esp_http_server.h>

namespace stuff {
namespace net {
namespace http {

#define _req (static_cast<httpd_req_t*>(_impl))
#define _req_httpd (static_cast<httpd_handle_t>(_server->_impl))

/**
 * @todo not tested
*/
bool Request::getHeader(const char *key, std::string *value) {
	auto &v = *value;
	auto len = ::httpd_req_get_hdr_value_len(_req, key);
	if (len <= 0) { return false; }
	v.resize(len+1);
	if (::httpd_req_get_hdr_value_str(_req, key, &v[0], len+1) != ESP_OK) {
		return false;
	}
	v.resize(len);
	return true;
}

int Request::read(void *buf, size_t len) {
	return ::httpd_req_recv(_req, static_cast<char*>(buf), len);
}

void Response::setStatus(status::Code code) {
	if (::httpd_resp_set_status(_req, status::Status(code)) != ESP_OK) {
		_failed = true;
	} else {
		_statusSet = true;
	}
}

void Response::setStatus(int code) {
	return setStatus(status::Code(code));
}

void Response::setHeader(const char *const &key, const char *const &value) {
	esp_err_t err;
	if (strcasecmp(key, "Content-Type") == 0) {
		err = ::httpd_resp_set_type(_req, value);
		if (err == ESP_OK) {
			_contentTypeSet = true;
		}
	} else {
		err = ::httpd_resp_set_hdr(_req, key, value);
	}
	if (err != ESP_OK) {
		_failed = true;
	}
}

static const char * defaultContentType = "text/plain; charset=utf-8";

int Response::write(const void *buf, size_t len) {
	if (len == 0 && !_isWritingChunkEnd) {
		return 0;
	}

	if (!_statusSet) {
		setStatus(status::Code::OK);
		_statusSet = true;
	}
	if (!_contentTypeSet) {
		setHeader("Content-Type", defaultContentType);
		_contentTypeSet = true;
	}

	if(::httpd_resp_send_chunk(_req, static_cast<const char*>(buf), len) != ESP_OK) {
		_failed = true;
	}
	
	return len;
}

int Response::write(const char *buf) {
	return write(buf, std::strlen(buf));
}

// 什么也不干，因为是 Server*。
static void _free_user_ctx(void *ctx) {}

#define _httpd (static_cast<httpd_handle_t>(_impl))

Server::Server()
	: Server(80)
{ }

Server::Server(uint16_t port)
{
	httpd_config_t cfg = HTTPD_DEFAULT_CONFIG();
	cfg.server_port = port;
	cfg.global_user_ctx = this;
	cfg.global_user_ctx_free_fn = _free_user_ctx;

	httpd_handle_t handle;
	if(::httpd_start(&handle, &cfg) != ESP_OK) {
		std::abort();
	}

	_impl = handle;
}
Server::~Server() {
	::httpd_stop(_httpd);
}

static httpd_method_t _method(Method::Value method) {
	switch(method) {
	case Method::Get:
		return HTTP_GET;
	case Method::Post:
		return HTTP_POST;
	default:
		std::abort();
	}
}

// Server::_handle 是私有方法，所以包装了一下。
class _CallServerHandle {
public:
	Server* s;
	bool operator()(std::function<HandlerFunc> h, Request &r, Response &w) {
		return s->_handle(h, r, w);
	}
};

struct _HandlerData {
	std::function<HandlerFunc> _h;
};

static esp_err_t _handler(httpd_req_t *req) {
	auto s = static_cast<Server*>(::httpd_get_global_user_ctx(req->handle));
	auto h = static_cast<_HandlerData*>(req->user_ctx);
	auto r = Request(s, req);
	auto w = Response(s, req);
	auto ok = _CallServerHandle{s}(h->_h, r, w);
	return ok ? ESP_OK : ESP_FAIL;
}

void Server::handle(Method::Value method, const char *path, std::function<HandlerFunc> handler) {
	httpd_uri_t uri = {
		.uri        = path,
		.method     = _method(method),
		.handler    = _handler,
		.user_ctx   = new _HandlerData{handler}, // TODO: 未释放
	};
	if (::httpd_register_uri_handler(_httpd, &uri) != ESP_OK) {
		std::abort();
	}
}

bool Server::_handle(std::function<HandlerFunc> h, Request &r, Response &w) {
	h(r, w);
	if (!w._failed) {
		w._isWritingChunkEnd = true;
		w.write("", 0); // 仍然会更新 _failed。
	}
	return !w._failed;
}

}
}
}

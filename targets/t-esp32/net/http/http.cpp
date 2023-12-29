#include <cstdlib>

#include "http.hpp"

#include <esp_http_server.h>

namespace stuff {
namespace net {
namespace http {

Server::Server() {
	httpd_config_t cfg = HTTPD_DEFAULT_CONFIG();
	httpd_handle_t handle;
	if(::httpd_start(&handle, &cfg) != ESP_OK) {
		std::abort();
	}
	_impl = handle;
}

#define _h (static_cast<httpd_handle_t>(_impl))

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

struct _HandlerWrapper {
	Server* server;
	HandlerFunc handler;
};

static esp_err_t _handler(httpd_req_t *req) {
	auto h = reinterpret_cast<_HandlerWrapper*>(req->user_ctx);
	auto r = Request(h->server, req);
	auto w = Response(h->server, req);
	h->handler(r, w);
}

void Server::handle(Method::Value method, const char *path, HandlerFunc handler) {
	// TODO delete w
	auto w = new _HandlerWrapper;
	w->server = this;
	w->handler = handler;
	httpd_uri_t uri = {
		.uri        = path,
		.method     = _method(method),
		.handler    = _handler,
		.user_ctx   = w,
	};
	if (::httpd_register_uri_handler(_h, &uri) != ESP_OK) {
		std::abort();
	}
}

std::tuple<Response, int> Server::get(Request &request) {
	
}

std::tuple<Response, int> Server::post(Request &request) {
	
}

}
}
}

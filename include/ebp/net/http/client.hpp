#pragma once

#include <ebp/base.hpp>

#include <tuple>
#include <memory>

#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <esp_event.h>
#include <esp_netif.h>
#include <esp_http_client.h>

#include "request.hpp"

namespace ebp {
namespace net {
namespace http {
	
class Request;
class Response;

class Client {
public:
	Client()
	{
		esp_http_client_config_t config;
		bzero(&config, sizeof(esp_http_client_config_t));
		config.event_handler = _eventHandler;
		config.user_data = this;
		config.url = "http://localhost";
		auto client = ::esp_http_client_init(&config);
		if (!client) { abort(); }
		_client = client;
	}
	virtual ~Client() {
		::esp_http_client_cleanup(_client);
	}
	
public:
	std::tuple<Response, esp_err_t>
	get(const std::string& url)
	{
		Request req(Method::HTTP_METHOD_GET, url);
		return roundTrip(req);
	}
	
	std::tuple<Response, esp_err_t>
	post(const std::string &url, const std::string &contentType, const std::string &body)
	{
		Request req(Method::HTTP_METHOD_POST, url);
		
		::esp_http_client_set_header(_client, "Content-Type", contentType.c_str());
		::esp_http_client_set_post_field(_client, body.c_str(), body.size());

		return roundTrip(req);
	}

protected:
	std::tuple<Response, esp_err_t>
	roundTrip(const Request& req)
	{
		::esp_http_client_set_url(_client, req._url.c_str());
		::esp_http_client_set_method(_client, req._method);

		auto err = ::esp_http_client_perform(_client);
		if (err != ESP_OK) {
			return { Response(*this), err };
		}

		return { Response(*this), ESP_OK };
	}
	
protected:
	static esp_err_t _eventHandler(esp_http_client_event_t *evt)
	{
		return reinterpret_cast<Client*>(evt->user_data)->eventHandler(evt);
	}
	esp_err_t eventHandler(esp_http_client_event_t *evt)
	{
		switch (evt->event_id) {
		default:
			puts("unhandled http event");
			break;
		case HTTP_EVENT_ERROR:
			puts("HTTP error");
			break;
		case HTTP_EVENT_ON_CONNECTED:
			break;
		case HTTP_EVENT_HEADERS_SENT:
			break;
		case HTTP_EVENT_ON_HEADER:
			printf("got header: %s: %s\n", evt->header_key, evt->header_value);
			break;
		case HTTP_EVENT_ON_DATA:
			break;
		case HTTP_EVENT_ON_FINISH:
			break;
		case HTTP_EVENT_DISCONNECTED:
			break;
		case HTTP_EVENT_REDIRECT:
			break;
		}
		
		return ESP_OK;
	}
	
protected:
	esp_http_client_handle_t _client;
};

} // namespace http
} // namespace net
} // namespace ebp

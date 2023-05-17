#pragma once

#include <ebp/base.hpp>

#include <tuple>
#include <memory>
#include <vector>
#include <unordered_map>

#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <esp_event.h>
#include <esp_netif.h>
#include <esp_http_client.h>

namespace ebp {
namespace http {

class Header
{
public:
	void clear()                                                { _headers.clear(); }
	std::size_t size() const                                    { return _headers.size(); }
	void set(const std::string &key, const std::string &value)  { return set(key.c_str(), value.c_str()); }
	void set(const char *key, const char *value)                { _headers[canonicalKey(std::string(key))] = value; }
	std::string get(const std::string&key) const                { return get(key.c_str()); }
	std::string get(const char *key) const {
		auto it = _headers.find(canonicalKey(std::string(key)));
		if (it == _headers.end()) { return ""; }
		return it->second;
	}
	bool remove(const std::string &key)                         { return remove(key.c_str()); }
	bool remove(const char *key)                                { return _headers.erase(canonicalKey(std::string(key))) > 0; }

public: // 迭代器支持
	auto begin()    const { return _headers.begin(); }
	auto end()      const { return _headers.end();   }
	
protected:
	// 把头部名字变成标准的名字。原地修改。
	static std::string canonicalKey(std::string&& key);
	// 判断是否是标准规定的头部名字字符集。
	static bool isToken(char c);

protected:
	// 目前仅支持单值，但是 HTTP 标准是明确支持同名 key 的。
	std::unordered_map<std::string, std::string> _headers;
};
	
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


class Response final {
public:
	Response(Client &client) : _client(client) {
	}
	~Response() {
	}

public:
	int statusCode() const;
	const Header &header() const;

	friend class Client;
protected:
	Client &_client;
};

class Client {
	friend class Request;
	friend class Response;
public:
	Client()
	{
		esp_http_client_config_t config;
		bzero(&config, sizeof(esp_http_client_config_t));
		config.event_handler = _eventHandler;
		config.user_data = this;
		config.url = "http://localhost";
		_client = alt::must(::esp_http_client_init(&config));
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
	esp_http_client_handle_t raw()
	{
		return _client;
	}

protected:
	std::tuple<Response, esp_err_t>
	roundTrip(const Request& req)
	{
		// 准备工作，清理上次的数据。
		_headers.clear();


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
			// printf("got header: %s: %s\n", evt->header_key, evt->header_value);
			_headers.set(evt->header_key, evt->header_value);
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
	Header _headers;
};

} // namespace http
} // namespace ebp

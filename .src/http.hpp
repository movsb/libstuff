#pragma once

#include <tuple>
#include <memory>
#include <vector>
#include <unordered_map>

#include <cstring>
#include <cstdlib>
#include <strings.h>

namespace stuff {
namespace net {
namespace http {

/**
 * @brief HTP Header 头部。
*/
class Header {
public:
	Header() {}
	Header(const Header&) = delete;
	Header(Header&& ref) {
		_pairs = std::move(ref._pairs);
	}
public:
	void clear()                                                { _pairs.clear(); }
	std::size_t size() const                                    { return _pairs.size(); }
	void set(const std::string &key, const std::string &value)  { return set(key.c_str(), value.c_str()); }
	void set(const char *key, const char *value)                { _pairs[canonicalKey(std::string(key))] = value; }
	std::string get(const std::string&key) const                { return get(key.c_str()); }
	std::string get(const char *key) const {
		auto it = _pairs.find(canonicalKey(std::string(key)));
		if (it == _pairs.end()) { return ""; }
		return it->second;
	}
	bool remove(const std::string &key)                         { return remove(key.c_str()); }
	bool remove(const char *key)                                { return _pairs.erase(canonicalKey(std::string(key))) > 0; }

public: // 迭代器支持
	auto begin()    const { return _pairs.begin(); }
	auto end()      const { return _pairs.end();   }
	
protected:
	// 把头部名字变成标准的名字。原地修改。
	static std::string canonicalKey(std::string&& key);
	// 判断是否是标准规定的头部名字字符集。
	static bool isToken(char c);

protected:
	// 目前仅支持单值，但是 HTTP 标准是明确支持同名 key 的。
	// 只是这种情况使用场景不是特别高，暂时不予以支持，后期可以支持。
	std::unordered_map<std::string, std::string> _pairs;
};

using Method = esp_http_client_method_t;

class Client {
public:
	// 很难定义在外面，因为要访问 client 内部的东西。。。
	// 定义友元也不好，很简单的函数都要写在 cpp 文件内，否则“定义不完整”，垃圾语言。
	// 后面使用 using 暴露到 http 域了。
	class Request {
		friend class Client;
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
		
		std::string     _body;
	};

	class Response final {
	public:
		Response(Client *client)
			: _client(client)
			, _plainBodyReader(client->raw())
			 { }
		~Response() {
			ESP_LOGI("http", "Response deconstructed");
		}
		Response(const Response &) = delete;
		Response(Response&& ref)
			: _client(ref._client)
			, _plainBodyReader(ref._client->raw())
			{
				ESP_LOGI("http", "move response construct");
				ref._client = nullptr;
			}
	
	private:
		class _PlainBodyReader : public io::Reader
		{
		public:
			_PlainBodyReader(esp_http_client_handle_t client)
				: _client(client) {}
				
		public:
			virtual std::tuple<int, esp_err_t> (read)(void *buf, size_t size) override {
				auto n = ::esp_http_client_read(_client, reinterpret_cast<char*>(buf), size);
				if (n < 0) {
					return { 0, n };
				} else if (n == 0) {
					return { 0, EOF };
				}
				return { n, ESP_OK };
			}
		private:
			esp_http_client_handle_t _client;
		};

	public:
		// 返回状态码。
		int statusCode() const {
			return ::esp_http_client_get_status_code(_client->raw());
		}
		// 返回头部字段列表。
		const Header& header() const {
			alt::must(_client);
			return _client->_pairs;
		}
		// 返回 body，作为 io::Reader 流。
		io::Reader& body();

	protected:
		friend class Client;
		Client              *_client;
		_PlainBodyReader    _plainBodyReader;
	};

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
		return execute(req);
	}
	
	std::tuple<Response, esp_err_t>
	post(const std::string &url, const std::string &contentType, const std::string &body)
	{
		Request req(Method::HTTP_METHOD_POST, url);
		
		::esp_http_client_set_header(_client, "Content-Type", contentType.c_str());
		::esp_http_client_set_post_field(_client, body.c_str(), body.size());

		return execute(req);
	}
	
protected:
	esp_http_client_handle_t raw()
	{
		return _client;
	}

protected:
	std::tuple<Response, esp_err_t> execute(const Request& req);
	std::tuple<Response, esp_err_t> roundTrip(const Request& req, const char* url);
	
protected:
	static esp_err_t _eventHandler(esp_http_client_event_t *evt)
	{
		return reinterpret_cast<Client*>(evt->user_data)->eventHandler(evt);
	}
	esp_err_t eventHandler(esp_http_client_event_t *evt);
	
protected:
	esp_http_client_handle_t    _client;
	Header                      _pairs;
};

using Request = Client::Request;
using Response = Client::Response;

} // namespace http
} // namespace net
} // namespace stuff

#include <cstring>
#include <strings.h>
#include <charconv>

#include "ebp/http.hpp"

namespace ebp {
namespace http {
	
std::string Header::canonicalKey(std::string&& key)
{
	for (auto &c : key) {
		// If key contains a space or invalid header field bytes, it is
		// returned without modifications.
		if (!isToken(c)) {
			return key;
		}
	}
	
	bool upper = true;
	constexpr auto diff = 'a' - 'A';

	for (auto &c : key) {
		// Canonicalize: first letter upper case
		// and upper case after each dash.
		// (Host, User-Agent, If-Modified-Since).
		// MIME headers are ASCII only, so no Unicode issues.
		if (upper && 'a' <= c && c <= 'z') {
			c -= diff;
		} else if (!upper && 'A' <= c && c <= 'Z') {
			c += diff;
		}
		upper = c == '-';
	}
	
	return key;
}

bool Header::isToken(char c)
{
	if ('0' <= c && c <= '9') return true;
	if ('a' <= c && c <= 'z') return true;
	if ('A' <= c && c <= 'Z') return true;

	switch (c) {
	case '!':
	case '#':
	case '$':
	case '%':
	case '&':
	case '\'':
	case '*':
	case '+':
	case '-':
	case '.':
	case '^':
	case '_':
	case '`':
	case '|':
	case '~':
		return true;
	}
	
	return false;
}


std::tuple<Response, esp_err_t> Client::roundTrip(const Request& req)
{
	Response rsp(*this);

	// 准备工作，清理上次的数据。
	// TODO：清理干净
	_headers.clear();

	::esp_http_client_set_url(_client, req._url.c_str());
	::esp_http_client_set_method(_client, req._method);
	
	// 打开连接，写入头部
	if (auto err = ::esp_http_client_open(_client, req._body.size()); err != ESP_OK) {
		return { rsp, err };
	}
	
	// 写入 body。没有的话，什么也不干。
	if (auto wn = ::esp_http_client_write(_client, req._body.c_str(), req._body.size()); wn != req._body.size()) {
		return { rsp, ESP_FAIL };
	}

	// 获取响应头部
	if (auto n = ::esp_http_client_fetch_headers(_client); n < 0) {
		return { rsp, ESP_FAIL };
	}
	
	rsp._length = static_cast<int>(::esp_http_client_get_content_length(_client));
	rsp._isChunked = ::esp_http_client_is_chunked_response(_client);

	return { rsp, ESP_OK };
}

io::Reader& Response::body()
{
	if (_length == 0 && !_isChunked) {
		return *io::EofReader;
	}
	if (_length > 0) {
		return _plainBodyReader;
	}
	if (_isChunked) {
		abort();
	}
	// 其实还有一种情况没处理：没有 content-length，也没有 chunked。很正常的一种情况。
	abort();
}

esp_err_t Client::eventHandler(esp_http_client_event_t *evt)
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

} // namespace http
} // namespace ebp

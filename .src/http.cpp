#include <cstring>
#include <strings.h>
#include <charconv>

#include "ebp/http.hpp"
#include "http.hpp"

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


std::tuple<Response, esp_err_t> Client::execute(const Request& req)
{
	static constexpr int maxRedirects = 10;
	
	std::string url(req._url);
	
	for (int nRedirects = 0; nRedirects < maxRedirects; nRedirects++) {
		auto [rsp, err] = roundTrip(req, url.c_str());
		if (err != ESP_OK) { return { std::move(rsp), err }; }
		switch (::esp_http_client_get_status_code(_client)) {
		default:
			return { std::move(rsp), err };
		case HttpStatus_MovedPermanently:
		case HttpStatus_Found:
		case HttpStatus_SeeOther:
		case HttpStatus_TemporaryRedirect:
		case HttpStatus_PermanentRedirect:
			url = rsp.header().get("Location");
			if (url.empty()) {
				ESP_LOGW("http", "invalid empty redirection url");
				return { Response(this), ESP_FAIL };
			}
			ESP_LOGI("http", "redirection: location=%s", url.c_str());
		}
	}
	return { Response(this), ESP_OK };
	// abort(); // TODO
}

std::tuple<Response, esp_err_t> Client::roundTrip(const Request& req, const char* url)
{
	// 准备工作，清理上次的数据。
	// TODO：清理干净
	_headers.clear();

	::esp_http_client_set_url(_client, url);
	::esp_http_client_set_method(_client, req._method);
	
	// 打开连接，写入头部
	if (auto err = ::esp_http_client_open(_client, req._body.size()); err != ESP_OK) {
		return { Response(this), err };
	}
	
	// 写入 body。没有的话，什么也不干。
	if (auto wn = ::esp_http_client_write(_client, req._body.c_str(), req._body.size()); wn != req._body.size()) {
		return { Response(this), ESP_FAIL };
	}

	// 获取响应头部
	if (auto n = ::esp_http_client_fetch_headers(_client); n < 0) {
		return { Response(this), ESP_FAIL };
	}
	
	return { Response(this), ESP_OK };
}

io::Reader& Response::body()
{
	auto length = static_cast<int>(::esp_http_client_get_content_length(_client->raw()));
	auto isChunked = ::esp_http_client_is_chunked_response(_client->raw());

	if (length == 0 && !isChunked) {
		return *io::EofReader;
	}
	if (length > 0) {
		return _plainBodyReader;
	}
	// http_parser 把 chunked 也处理了（为了知道 body 结束了）。
	// 所以这里的 plain 也可以用于读取 chunked body 🥵
	if (isChunked) {
		return _plainBodyReader;
	}
	// 其实还有一种情况没处理：没有 content-length，也没有 chunked。很正常的一种情况。
	// 但是好像 esp 把它当成 chunked 处理？有点不规范。。。
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
		ESP_LOGI("http", "connected");
		break;
	case HTTP_EVENT_HEADERS_SENT:
		ESP_LOGI("http", "headers sent");
		break;
	case HTTP_EVENT_ON_HEADER:
		// printf("got header: %s: %s\n", evt->header_key, evt->header_value);
		_headers.set(evt->header_key, evt->header_value);
		break;
	case HTTP_EVENT_ON_DATA:
		ESP_LOGI("http", "data");
		break;
	case HTTP_EVENT_ON_FINISH:
		ESP_LOGI("http", "finished");
		break;
	case HTTP_EVENT_DISCONNECTED:
		ESP_LOGI("http", "disconnected");
		break;
	case HTTP_EVENT_REDIRECT:
		ESP_LOGI("http", "need redirection");
		break;
	}
	
	return ESP_OK;
}

} // namespace http
} // namespace ebp

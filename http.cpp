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

int Response::statusCode() const {
	return ::esp_http_client_get_status_code(_client.raw());
}
const Header& Response::header() const {
	return _client._headers;
}

} // namespace http
} // namespace ebp

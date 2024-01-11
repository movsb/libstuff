#include <stuff/net/url.hpp>

#include <tuple>
#include <string>
#include <cstring>

namespace stuff {
namespace net {
namespace url {

enum class _Encoding {
	queryComponent,
};

static bool isHex(const char c) {
	return (c >= '0' && c <= '9')
		|| (c >= 'a' && c <= 'f')
		|| (c >= 'A' && c <= 'F')
		;
}

static uint8_t unHex(const char c) {
	if (c >= '0' && c <= '9') {
		return (uint8_t)(c - '0');
	} else if (c >= 'a' && c <= 'f') {
		return (uint8_t)(c - 'a' + 10);
	} else if (c >= 'A' && c <= 'F') {
		return (uint8_t)(c - 'A' + 10);
	}
	return 0;
}

/**
 * @brief 解码转义字符串。
 * @see /go/1.21.5/libexec/src/net/url/url.go
*/
static std::tuple<std::string, bool> unescape(const char *s, const char *e, _Encoding mode) {
	if (mode != _Encoding::queryComponent) {
		std::abort();
	}

	std::string buf;
	const auto len = e - s;
	buf.reserve(len);
	
	for (int i = 0; i < len; i++) {
		switch (s[i]) {
		case '%':
			if (i+2 < len && isHex(s[i+1]) && isHex(s[i+2])) {
				buf += unHex(s[i+1])<<4 | unHex(s[i+2]);
				i += 2;
				continue;
			}
			return {{}, false};
		case '+':
			buf += mode == _Encoding::queryComponent ? ' ' : '+';
			break;
		default:
			buf += s[i];
		}
	}

	return { std::move(buf), true };
}

static std::tuple<std::string, bool> queryUnescape(const char *s, const char *e) {
	return unescape(s, e, _Encoding::queryComponent);
}

std::tuple<Values, bool> parseQuery(const char *query) {
	auto values = Values{};

	if (!query || !*query) {
		return {std::move(values), true};
	}

	const char *p = query;
	const char * const q = query + std::strlen(query);

	static auto const _index = [](const char *s, const char *e, int c) {
		return static_cast<const char *>(std::memchr(s, c, e - s));
	};

	while (*p) {
		const char *kvs = p, *kve;
		if (auto amp = _index(p, q, '&'); amp) {
			kve = amp;
			p = amp+1;
		} else {
			kve = q;
			p = q;
		}
		
		// 空的 key 是合法的。
		if (kvs == kve) { continue; }
		// TODO 处理 ; （不支持，太老了）

		const char *ks = kvs, *ke, *vs, *ve = kve;
		if (auto equ = _index(kvs, kve, '='); equ) {
			ke = equ;
			vs = equ+1;
		} else {
			ke = kve;
			vs = kve;
		}

		auto [key, ok1] = queryUnescape(ks, ke);
		auto [val, ok2] = queryUnescape(vs, ve);
		if (!ok1 || !ok2) {
			return { std::move(values), false };
		}
		values.set(std::move(key), std::move(val));
	}

	return { std::move(values), true };
}

}
}
}

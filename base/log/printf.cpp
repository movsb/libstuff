#include "printf.hpp"
#include <inttypes.h>
#include <array>

extern "C" int write(int fd, const char* buf, int size);

namespace stuff {
namespace base {
namespace alts {

static inline int _outputChar(char c) {
	return write(1, &c, 1);
}

int _outputStr(const char* s) {
	int n = 0;
	while(*s) {
		n += _outputChar(*s);
		s++;
	}
	return n;
}

static const char* const lowerDigits = "0123456789abcdef";
static const char* const upperDigits = "0123456789ABCDEF";

static int _formatBits(char (&buf)[1+64+1], uint64_t u, uint8_t base, bool neg, const char* const digits) {
	auto &a = buf;

	uint8_t i = std::size(buf);
	i--;
	a[i] = '\0';

	if (neg) u = -u;
	
	auto b = uint64_t(base);
	while (u >= b) {
		i--;
		auto q = u / b;
		a[i] = digits[u-q*b];
		u = q;
	}
	// u < base
	i--;
	a[i] = digits[u];
	
	if (neg) {
		i--;
		a[i] = '-';
	}
	
	return i;
}

static int _outputNumber(uint64_t u, uint8_t base, bool neg, const char* digits) {
	char buf[66];
	auto p = _formatBits(buf, u, base, neg, digits);
	return _outputStr(&buf[p]);
}

int _skip2percent(const char* &fmt) {
	int n = 0;
	while(*fmt && *fmt != '%') {
		n += _outputChar(*fmt);
		fmt++;
	}
	if (*fmt == '%') { fmt++; }
	if (*fmt == '%') {
		fmt++;
		n += _outputChar('%');
		return n + _skip2percent(fmt);
	}
	return n;
}

int _unknown(char c) {
	int n = _outputChar('%');
	n += _outputStr("<unknown>");
	n += _outputChar(c);
	return n;
} 

int _printf(const char *&fmt, bool b) {
	int n = _skip2percent(fmt);
	switch (*fmt) {
	case 'v':
	case 't':
		fmt++;
		// fallthrough
	case 0:
		return n + _outputStr(b ? "true" : "false");
	case 'd':
		fmt++;
		return n + _outputStr(b ? "1" : "0");
	default:
		return n + _unknown(*fmt++);
	}
}

int _printf(const char *&fmt, uint64_t u, bool neg) {
	int n = _skip2percent(fmt);
	switch (*fmt) {
	case 'v':
	case 'd':
		fmt++;
		// fallthrough
	case 0:
		return _outputNumber(u, 10, neg, lowerDigits);
	case 'b':
		fmt++;
		return _outputNumber(u, 2, neg, lowerDigits);
	case 'x':
		fmt++;
		return _outputNumber(u, 16, neg, lowerDigits);
	case 'X':
		fmt++;
		return _outputNumber(u, 16, neg, upperDigits);
	default:
		return n + _unknown(*fmt++);
	}
}

int _printf(const char *&fmt, signed long long int i) {
	return _printf(fmt, static_cast<uint64_t>(i), i < 0);
}

int _printf(const char *&fmt, unsigned long long int u) {
	return _printf(fmt, static_cast<uint64_t>(u), false);
}

int _printf(const char *&fmt, const char *s) {
	int n = _skip2percent(fmt);
	switch (*fmt) {
	case 's':
		fmt++;
		// fallthrough
	case 0:
		n += _outputStr(s);
		return n;
	default:
		return n + _unknown(*fmt++);
	}
}

int _printf(const char *&fmt, const void *p) {
	int n = _skip2percent(fmt);
	switch(*fmt) {
		case 'p':
			fmt++;
			// fallthrough
		case 0: {
			char buf[66];
			auto i = _formatBits(buf, reinterpret_cast<uint64_t>(p), 16, false, upperDigits);
			// 计算当前位数，并默认总是补全 0
			auto nd = std::size(buf) - i - 1;
			while(nd < sizeof(p) * 2) {
				buf[--i] = '0';
				nd++;
			}
			buf[--i] = 'x';
			buf[--i] = '0';
			return _outputStr(&buf[i]);
		}
		default:
			return n + _unknown(*fmt++);
	}
}

} // namespace alts
} // namespace base
} // namespace stuff

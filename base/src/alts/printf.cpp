#include <stuff/base/alts/printf.hpp>
#include <cstdio>

namespace stuff {
namespace base {
namespace alts {

int _outputChar(char c) {
	return ::putchar(c);
}
int _outputStr(const char* s) {
	return ::printf("%s", s);
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

int _printf(const char *&fmt, char c) {
	int n = _skip2percent(fmt);
	switch (*fmt) {
	case 'v':
	case 'c':
		fmt++;
		// fallthrough
	case 0:
		return n + _outputChar(c);
	case 'd':
		fmt--;
		return n + _printf(fmt, static_cast<int64_t>(c));
	default:
		return n + _unknown(*fmt++);
	}
}

int _printf(const char *&fmt, int64_t i) {
	int n = _skip2percent(fmt);
	switch (*fmt) {
	case 'v':
	case 'd':
		fmt++;
		// fallthrough
	case 0:
		if (i == 0) {
			n += _outputChar('0');
			return n;
		}
		if (i < 0) {
			n += _outputChar('-');
			// BUG: 没处理 -max 的情况
			i = -i;
		}
		while(i > 0) {
			int r = i % 10;
			n += _outputChar('0' + r);
			i /= 10;
		}
		return n;
	default:
		return n + _unknown(*fmt++);
	}
}
int _printf(const char *&fmt, uint64_t i) {
	int n = _skip2percent(fmt);
	switch (*fmt) {
	case 'v':
	case 'd':
		fmt++;
		// fallthrough
	case 0:
		if (n == 0) {
			n += _outputChar('0');
			return n;
		}
		while(i > 0) {
			int r = i % 10;
			n += _outputChar('0' + r);
			i /= 10;
		}
		return n;
	default:
		return n + _unknown(*fmt++);
	}
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

} // namespace alts
} // namespace base
} // namespace stuff

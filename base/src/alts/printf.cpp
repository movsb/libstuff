#include <stuff/base/alts/printf.hpp>
#include <cstdio>
#include <inttypes.h>

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
	int msb;

	int n = _skip2percent(fmt);
	switch (*fmt) {
	case 'v':
	case 'd':
		fmt++;
		// fallthrough
	case 0:
		return ::printf("%" PRId64, i);
	case 'x':
		fmt++;
		return ::printf("%" PRIx64, i);
	case 'X':
		fmt++;
		return ::printf("%" PRIX64, i);
	case 'b':
		fmt++;
		if (i < 0) {
			n += _outputChar('-');
			i = -i;
		}
		msb = 62;
		for (; msb >= 1; msb--) {
			if ((i & (int64_t(1) << msb)) == 0) {
				continue;
			}
			break;
		}
		for (; msb >= 1; msb--) {
			char c = (i & (int64_t(1) << msb)) == 0 ? '0' : '1';
			n += _outputChar(c);
		}
		n += _outputChar(i & 1 ? '1' : '0');
		return n;
	default:
		return n + _unknown(*fmt++);
	}
}
int _printf(const char *&fmt, uint64_t i) {
	int msb;
	int n = _skip2percent(fmt);
	switch (*fmt) {
	case 'v':
	case 'd':
		fmt++;
		// fallthrough
	case 0:
		return ::printf("%" PRIu64, i);
	case 'x':
		fmt++;
		return ::printf("%" PRIx64, i);
	case 'X':
		fmt++;
		return ::printf("%" PRIX64, i);
	case 'b':
		fmt++;
		if (i < 0) {
			n += _outputChar('-');
			i = -i;
		}
		msb = 63;
		for (; msb >= 1; msb--) {
			if ((i & (uint64_t(1) << msb)) == 0) {
				continue;
			}
			break;
		}
		for (; msb >= 1; msb--) {
			char c = (i & (uint64_t(1) << msb)) == 0 ? '0' : '1';
			n += _outputChar(c);
		}
		n += _outputChar(i & 1 ? '1' : '0');
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

int _printf(const char *&fmt, float i) {
	int n = _skip2percent(fmt);
	switch (*fmt) {
	case 'f':
		fmt++;
		// fallthrough
	case 0:
		return ::printf("%f", i);
	default:
		return n + _unknown(*fmt++);
	}
}
int _printf(const char *&fmt, double i) {
	int n = _skip2percent(fmt);
	switch (*fmt) {
	case 'f':
		fmt++;
		// fallthrough
	case 0:
		return ::printf("%l", i);
	default:
		return n + _unknown(*fmt++);
	}
}

} // namespace alts
} // namespace base
} // namespace stuff

#include "printf.hpp"
#include <inttypes.h>
#include <array>
#include <assert.h>
#include <cstring>

/**
 * @note 所有 case 0 的地方，是为了打印没有格式化/格式化控制符少于参数个数的那些参数的。
 *          目前已经按默认格式仍然输出。
*/

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

// 2023 年每个软件开发者都必须知道的关于 Unicode 的最基本的知识（仍然不准找借口！）
// https://blog.xinshijiededa.men/unicode/
// https://tonsky.me/blog/unicode/
//
// https://en.cppreference.com/w/cpp/language/character_literal
/*
U+0000..007F    0xxxxxxx
U+0080..07FF    110xxxxx    10xxxxxx
U+0800..FFFF    1110xxxx    10xxxxxx    10xxxxxx
U+10000..10FFFF 11110xxx    10xxxxxx    10xxxxxx    10xxxxxx
*/
static int _outputRune(uint64_t r) {
	unsigned char buf[4+1];

	if (r <= 0x7F) {
		buf[0] = r;
		buf[1] = 0;
	} else if (r <= 0x07FF) {
		buf[0] = 0b11000000 | (r >> 6           );
		buf[1] = 0b10000000 | (r >> 0 & 0b111111);
		buf[2] = 0;
	} else if (r <= 0xFFFF) {
		buf[0] = 0b11100000 | (r >> 12          );
		buf[1] = 0b10000000 | (r >> 6 & 0b111111);
		buf[2] = 0b10000000 | (r >> 0 & 0b111111);
		buf[3] = 0;
	} else if (r <= 0x10FFFF) {
		buf[0] = 0b11110000 | (r >> 18          );
		buf[1] = 0b10000000 | (r >> 12 & 0b111111);
		buf[2] = 0b10000000 | (r >> 6  & 0b111111);
		buf[3] = 0b10000000 | (r >> 0  & 0b111111);
		buf[4] = 0;
	} else {
		// 其它无效字符替换成替换字符（Replacement Character）。
		// U+FFFD, {EF, BF, BD}
		// return _outputRune(U'�');
		buf[0] = 0xEF;
		buf[1] = 0xBF;
		buf[2] = 0xBD;
		buf[3] = 0;
	}

	return _outputStr(reinterpret_cast<const char*>(buf));
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

int _printf(const char *&fmt, char32_t c) {
	int n = _skip2percent(fmt);
	switch (*fmt) {
	case 'v':
	case 'c':
		fmt++;
		// fallthrough
	case 0:
		return n + _outputRune(static_cast<uint64_t>(c));
	default:
		fmt--;
		return n + _printf(fmt, static_cast<uint64_t>(c), false);
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
	case 'o':
		fmt++;
		return _outputNumber(u, 8, neg, lowerDigits);
	case 'x':
		fmt++;
		return _outputNumber(u, 16, neg, lowerDigits);
	case 'X':
		fmt++;
		return _outputNumber(u, 16, neg, upperDigits);
	case 'c':
		fmt++;
		return _outputRune(u);
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
	case 'v':
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
		case 'v':
		case 'p':
			fmt++;
			// fallthrough
		case 0:
		{
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

// https://gist.github.com/JBlond/2fea43a3049b38287e5e9cefc87b2124
#define __CSI_BEG   "\033["
#define __CSI_END   "m"
#define __CSI_RST   (__CSI_BEG "0" __CSI_END)

#define __BLACK     "30"
#define __RED       "31"
#define __GREEN     "32"
#define __YELLOW    "33"
#define __BLUE      "34"
#define __PURPLE    "35"
#define __CYAN      "36"
#define __WHITE     "37"

#define __RESET     "0"
#define __BOLD      "1"
#define __UNDERLINE "4"
#define __DELIM     ";"

int _printf(const char* & /* fmt */, const style::Style &s) {
	if (s.f & style::__empty_format && s.c == style::__empty_color) {
		return 0;
	}

	if (s.f & style::reset) {
		return _outputStr(__CSI_RST);
	}

	char buf[64] = { __CSI_BEG };
	bool first = true;
	auto cat = [&](const char* s) {
		if (first) {
			first = false;
		} else {
			std::strcat(buf, __DELIM);
		}
		std::strcat(buf, s);
	};

	// std::strcat(buf, __RESET);

	if (s.f & style::bold) {
		cat(__BOLD);
	}
	if (s.f & style::underline) {
		cat(__UNDERLINE);
	}

	switch (s.c) {
	case style::black:  cat(__BLACK); break;
	case style::white:  cat(__WHITE); break;
	case style::red:    cat(__RED);   break;
	case style::green:  cat(__GREEN); break;
	case style::yellow: cat(__YELLOW);break;
	case style::blue:   cat(__BLUE);  break;
	case style::purple: cat(__PURPLE);break;
	case style::cyan:   cat(__CYAN);  break;
	default: break;
	}
	
	std::strcat(buf, __CSI_END);
	
	// just in case
	assert(std::strlen(buf) < std::size(buf));

	return _outputStr(buf);
}

} // namespace alts
} // namespace base
} // namespace stuff

#include <utility>
#include <type_traits>

#include <stdint.h>
#include <stdio.h>

/**
 * @brief 原地替换 printf 的一种强类型实现。接收任意个数的参数并输出。
 * 
 * 强类型的好处之一是：打印 int、int16、int32、int64 不用再区分 %d、%ld、%u 之类的了，可以统一成 %d。
 * 另外一个好处是：可以打印对象（比如包含 toString 的对象）。
 * 
 * 支持的类型：
 * 
 *   - bool
 *   - int, unsigned int
 *   - int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t
 *   - char* 输出为字符串
 *   - 其它任何指针（输出地址）
 *   - 带 `const char* toString()` 方法的结构体或类
 *
 * 支持的格式：
 * 
 *   常规：
 * 
 *   - %% 字面值 %
 *   - %v 值的默认格式
 *   - %t 布尔值：true / false
 *   
 *   整数：
 * 
 *   - %b 整数的二进制 - todo
 *   - %c unicode 码点值 - todo
 *   - %d 整数的十进制
 *   - %o 整数的八进制 - todo
 *   - %x 十六进制（小写）- todo
 *   - %X 十六进制（大写） - todo
 *   
 *   浮点数：
 * 
 *   （待支持）
 * 
 *   字符串/字节数组：
 *   
 *   - %s 字符串
 *   - %X - todo 
 *   - %x - todo
 * 
 *   指针：
 * 
 *   - %d
 *   - %p 0x 表示的十六进制 - todo
*/

int outputChar(char c) {
	return putchar(c);
}
int outputStr(const char* s) {
	return printf("%s", s);
}

/**
 * 实现了一个强类型的输出库。也不知道有没有什么用？
*/

namespace stuff {
namespace base {
namespace alts {

int _printf(const char *&fmt, int64_t i);

int skip2percent(const char* &fmt) {
	int n = 0;
	while(*fmt && *fmt != '%') {
		n += outputChar(*fmt);
		fmt++;
	}
	if (*fmt == '%') { fmt++; }
	if (*fmt == '%') {
		fmt++;
		n += outputChar('%');
		return n + skip2percent(fmt);
	}
	return n;
}

int unknown(char c) {
	int n = outputChar('%');
	n += outputStr("<unknown>");
	n += outputChar(c);
	return n;
} 

int _printf(const char *&fmt, bool b) {
	int n = skip2percent(fmt);
	switch (*fmt) {
	case 'v':
	case 't':
		fmt++;
		// fallthrough
	case 0:
		return n + outputStr(b ? "true" : "false");
	case 'd':
		fmt++;
		return n + outputStr(b ? "1" : "0");
	default:
		return n + unknown(*fmt++);
	}
}

int _printf(const char *&fmt, char c) {
	int n = skip2percent(fmt);
	switch (*fmt) {
	case 'v':
	case 'c':
		fmt++;
		// fallthrough
	case 0:
		return n + outputChar(c);
	case 'd':
		fmt--;
		return n + _printf(fmt, static_cast<int64_t>(c));
	default:
		return n + unknown(*fmt++);
	}
}

int _printf(const char *&fmt, int64_t i) {
	int n = skip2percent(fmt);
	switch (*fmt) {
	case 'v':
	case 'd':
		fmt++;
		// fallthrough
	case 0:
		if (i == 0) {
			n += outputChar('0');
			return n;
		}
		if (i < 0) {
			n += outputChar('-');
			// BUG: 没处理 -max 的情况
			i = -i;
		}
		while(i > 0) {
			int r = i % 10;
			n += outputChar('0' + r);
			i /= 10;
		}
		return n;
	default:
		return n + unknown(*fmt++);
	}
}
int _printf(const char *&fmt, uint64_t i) {
	int n = skip2percent(fmt);
	switch (*fmt) {
	case 'v':
	case 'd':
		fmt++;
		// fallthrough
	case 0:
		if (n == 0) {
			n += outputChar('0');
			return n;
		}
		while(i > 0) {
			int r = i % 10;
			n += outputChar('0' + r);
			i /= 10;
		}
		return n;
	default:
		return n + unknown(*fmt++);
	}
}

int _printf(const char *&fmt, int8_t i)       { return _printf(fmt, static_cast<int64_t>(i));  }
int _printf(const char *&fmt, uint8_t i)      { return _printf(fmt, static_cast<uint64_t>(i)); }
int _printf(const char *&fmt, int16_t i)      { return _printf(fmt, static_cast<int64_t>(i));  }
int _printf(const char *&fmt, uint16_t i)     { return _printf(fmt, static_cast<uint64_t>(i)); }
int _printf(const char *&fmt, int32_t i)      { return _printf(fmt, static_cast<int64_t>(i));  }
int _printf(const char *&fmt, uint32_t i)     { return _printf(fmt, static_cast<uint64_t>(i)); }
int _printf(const char *&fmt, int i)          { return _printf(fmt, static_cast<int64_t>(i));  }
int _printf(const char *&fmt, unsigned int i) { return _printf(fmt, static_cast<uint64_t>(i)); }

int _printf(const char *&fmt, const char *s) {
	int n = skip2percent(fmt);
	switch (*fmt) {
	case 's':
		fmt++;
		// fallthrough
	case 0:
		n += outputStr(s);
		return n;
	default:
		return n + unknown(*fmt++);
	}
}

template<typename T>
int _printf(const char* &fmt, T *t) {
	int n = skip2percent(fmt);
	switch (*fmt) {
	case 'd':
		fmt--;
		// fallthrough
	case '0':
		return n + _printf(fmt, reinterpret_cast<uint64_t>(t));
	default:
		return n + unknown(*fmt++);
	}
}

template <typename T>
struct _has_to_string {
private:
    template <typename C> static constexpr auto check(C*) -> typename std::is_same<decltype(std::declval<C>().toString()), const char*>::type;
    template <typename>   static constexpr std::false_type check(...);
public:
    static constexpr bool value = std::is_same<decltype(check<T>(nullptr)), std::true_type>::value;
};

template <typename T>
typename std::enable_if<_has_to_string<T>::value, int >::type
_printf(const char* &fmt, const T &t) {
	int n = skip2percent(fmt);
	switch (*fmt) {
	case 's':
		fmt--;
		// fallthrough
	case 0:
		return n + _printf(fmt, t.toString());
	default:
		return n + unknown(*fmt++);
	}
}

template<typename F, typename... Args>
int _printf(const char* &fmt, const F& first, Args&&... args) {
	int n = _printf(fmt, first);
	n += _printf(fmt, std::forward<Args&&>(args)...);
	return n;
}
template<typename... Args>
int printf(const char* fmt, Args&&... args) {
	int n = _printf(fmt, std::forward<Args&&>(args)...);
	if (*fmt) {
		const char* remain = "%s";
		n += _printf(remain, fmt);
	}
	return n;
}

} // namespace alts
} // namespace base
} // namespace stuff


#if 0

using namespace stuff::base::alts;

int main() {
		// 假设有一个需要 toString 函数的类
	struct MyClass {
		const char* toString() const {
			return "my class";
		}
	};

    MyClass obj;
	stuff::base::alts::printf("%t %% %d %d %d %s %s %c %f\n", false, 1, 1, 1, "1234", obj, 'c', 3.14);
}

#endif

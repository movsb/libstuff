#pragma once

#include <utility>
#include <type_traits>

#include <stdint.h>

/**
 * @brief 替换 printf 的一种强类型实现。接收任意个数的参数并输出。
 * 
 * 强类型的好处之一是：打印 int、int8、int16、int32、int64 不用再区分 %d、%ld、%u %llu 之类的了，可以统一成 %d。
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
 *   - %d 整数的十进制
 *   - %b 整数的二进制 - todo
 *   - %c unicode 码点值 - todo
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

namespace stuff {
namespace base {
namespace alts {

int _outputStr(const char* s);

int _printf(const char *&fmt, int64_t i);

int _skip2percent(const char* &fmt);
int _unknown(char c);
int _printf(const char *&fmt, bool b);
int _printf(const char *&fmt, char c);
int _printf(const char *&fmt, int64_t i);
int _printf(const char *&fmt, uint64_t i);

inline int _printf(const char *&fmt, int8_t i)       { return _printf(fmt, static_cast<int64_t>(i));  }
inline int _printf(const char *&fmt, uint8_t i)      { return _printf(fmt, static_cast<uint64_t>(i)); }
inline int _printf(const char *&fmt, int16_t i)      { return _printf(fmt, static_cast<int64_t>(i));  }
inline int _printf(const char *&fmt, uint16_t i)     { return _printf(fmt, static_cast<uint64_t>(i)); }
inline int _printf(const char *&fmt, int32_t i)      { return _printf(fmt, static_cast<int64_t>(i));  }
inline int _printf(const char *&fmt, uint32_t i)     { return _printf(fmt, static_cast<uint64_t>(i)); }

inline int _printf(const char *&fmt, int i)          { return _printf(fmt, static_cast<int64_t>(i));  }
inline int _printf(const char *&fmt, unsigned int i) { return _printf(fmt, static_cast<uint64_t>(i)); }

// int _printf(const char *&fmt, float i);
// int _printf(const char *&fmt, double i);

int _printf(const char *&fmt, const char *s);

template<typename T>
int _printf(const char* &fmt, T *t) {
	int n = _skip2percent(fmt);
	switch (*fmt) {
	case 'd':
		fmt--;
		// fallthrough
	case '0':
		return n + _printf(fmt, reinterpret_cast<uint64_t>(t));
	default:
		return n + _unknown(*fmt++);
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
	int n = _skip2percent(fmt);
	switch (*fmt) {
	case 's':
		fmt--;
		// fallthrough
	case 0:
		return n + _printf(fmt, t.toString());
	default:
		return n + _unknown(*fmt++);
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
template<>
inline int printf(const char* s) {
	return _outputStr(s);
}

} // namespace alts
} // namespace base
} // namespace stuff

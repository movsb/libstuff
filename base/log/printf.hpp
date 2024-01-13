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
 * 类型支持：
 * 
 *   - [X] bool
 *   - [X] char, unsigned char, char32_t
 *   - [X] int, unsigned int
 *   - [X] int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t
 *   - [X] char* 输出为字符串
 *   - [X] 其它任何指针（输出地址）
 *   - [X] 带 `const char* toString()` 方法的结构体或类
 *
 * 支持的格式：
 * 
 *   常规：
 * 
 *   - [X] %% 字面值 %
 *   - [X] %v 值的默认格式
 *
 *   布尔（%v = %t）：
 *
 *   - [X] %t 布尔值：true / false
 *   - [X] %v = %t
 * 
 *   字符（%v = %c）：
 * 
 *   - [X] %c 对应的 Unicode 符号。
 * 
 *   整数（%v = %d）：
 * 
 *   - [X] %b 整数的二进制
 *   - [ ] %o 整数的八进制
 *   - [X] %d 整数的十进制
 *   - [X] %x 十六进制（小写）
 *   - [X] %X 十六进制（大写）
 *   - [X] %c Unicode 码点值
 *   
 *   浮点数：
 * 
 *   - [ ] %f
 * 
 *   字符串（%v = %s）：
 *
 *   - [X] %s 字符串
 *   - [ ] %X
 *   - [ ] %x
 * 
 *   指针（%v = %p）：
 * 
 *   - [X] %p 0x 表示的十六进制，固定显示为指针长度，大写字母。
 * 
 * @todo 整理单元测试。
*/

namespace stuff {
namespace base {
namespace alts {

int _outputStr(const char* s);
int _skip2percent(const char* &fmt);
int _unknown(char c);

int _printf(const char *&fmt, bool b);
int _printf(const char *&fmt, char32_t c);
int _printf(const char *&fmt,   signed long long int i);
int _printf(const char *&fmt, unsigned long long int u);
int _printf(const char *&fmt, const char *s);
int _printf(const char *&fmt, const void *p);

inline int _printf(const char *&fmt, char               c) { return _printf(fmt, static_cast<char32_t>(c));                }
inline int _printf(const char *&fmt,   signed char      i) { return _printf(fmt, static_cast<  signed long long int>(i));  }
inline int _printf(const char *&fmt, unsigned char      u) { return _printf(fmt, static_cast<  signed long long int>(u));  }
inline int _printf(const char *&fmt,   signed short int i) { return _printf(fmt, static_cast<  signed long long int>(i));  }
inline int _printf(const char *&fmt, unsigned short int u) { return _printf(fmt, static_cast<unsigned long long int>(u));  }
inline int _printf(const char *&fmt,   signed int       i) { return _printf(fmt, static_cast<  signed long long int>(i));  }
inline int _printf(const char *&fmt, unsigned int       u) { return _printf(fmt, static_cast<unsigned long long int>(u));  }
inline int _printf(const char *&fmt,   signed long int  i) { return _printf(fmt, static_cast<  signed long long int>(i));  }
inline int _printf(const char *&fmt, unsigned long int  u) { return _printf(fmt, static_cast<unsigned long long int>(u));  }

template<typename T>
int _printf(const char* &fmt, T *t) {
	return _printf(fmt, static_cast<const void*>(t));
}
inline int _printf(const char *&fmt, std::nullptr_t) {
	return _printf(fmt, static_cast<const void*>(nullptr));
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
	return _printf(fmt, t.toString());
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
		n += _outputStr(fmt);
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

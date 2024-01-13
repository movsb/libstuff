#pragma once

#include <utility>
#include <type_traits>

#include <stdint.h>

/**
 * @brief 替换 printf 的一种强类型实现。接收任意个数的参数并输出。
 *
 * 别看重载函数非常多，其实只对外暴露了一个函数：
 * 
 *     template<typename... Args>
 *     int printf(const char* fmt, Args&&... args);
 *
 * 以及一些预定义的样式：Black, Red, Green...
 *
 * 所有以下划线开头的函数（比如：_printf）外部均不可使用。
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
 * 格式支持：
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
 *   - [o] %o 整数的八进制
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
 * 样式支持：
 * 
 * - 颜色
 * - 字体
 * 
 * @todo 整理单元测试。
 * @bug 不支持类似 %[0]s 的表示。因为实现方式使用了 C++ 的模板展开，无法提前和延迟获取到顺序不一致的参数。无解，但是很少被人知道/使用。
 * @bug 多次 printf 调用之间的样式无法嵌套。
*/

namespace stuff {
namespace base {
namespace alts {

/**
 * @brief 一些辅助函数。
*/
///@{
int _outputStr(const char* s);
int _skip2percent(const char* &fmt);
int _unknown(char c);
///@}

/**
 * @brief 布尔、字符、整型、字符串、指针。
*/
///@{
int _printf(const char *&fmt, bool b);
int _printf(const char *&fmt, char32_t c);
int _printf(const char *&fmt,   signed long long int i);
int _printf(const char *&fmt, unsigned long long int u);
int _printf(const char *&fmt, const char *s);
int _printf(const char *&fmt, const void *p);
///@}

/**
 * @brief 不同宽度的字符、整型处理函数。
*/
///@{
inline int _printf(const char *&fmt, char               c) { return _printf(fmt, static_cast<char32_t>(c));                }
inline int _printf(const char *&fmt,   signed char      i) { return _printf(fmt, static_cast<  signed long long int>(i));  }
inline int _printf(const char *&fmt, unsigned char      u) { return _printf(fmt, static_cast<  signed long long int>(u));  }
inline int _printf(const char *&fmt,   signed short int i) { return _printf(fmt, static_cast<  signed long long int>(i));  }
inline int _printf(const char *&fmt, unsigned short int u) { return _printf(fmt, static_cast<unsigned long long int>(u));  }
inline int _printf(const char *&fmt,   signed int       i) { return _printf(fmt, static_cast<  signed long long int>(i));  }
inline int _printf(const char *&fmt, unsigned int       u) { return _printf(fmt, static_cast<unsigned long long int>(u));  }
inline int _printf(const char *&fmt,   signed long int  i) { return _printf(fmt, static_cast<  signed long long int>(i));  }
inline int _printf(const char *&fmt, unsigned long int  u) { return _printf(fmt, static_cast<unsigned long long int>(u));  }
///@}

/**
 * @brief 任意指针、空指针处理。
*/
///@{
template<typename T>
int _printf(const char* &fmt, T *t) {
	return _printf(fmt, static_cast<const void*>(t));
}
inline int _printf(const char *&fmt, std::nullptr_t) {
	return _printf(fmt, static_cast<const void*>(nullptr));
}
///@}

/**
 * @brief 带 toString() 方法的类的处理。
*/
///@{
template <typename T>
struct _has_to_string {
private:
    template <typename C> static constexpr auto check(C*) -> typename std::is_same<decltype(std::declval<C>().toString()), const char*>::type;
    template <typename>   static constexpr std::false_type check(...);
public:
    static constexpr bool value = std::is_same<decltype(check<T>(nullptr)), std::true_type>::value;
};

template <typename T>
typename std::enable_if<_has_to_string<T>::value, int>::type
_printf(const char* &fmt, const T &t) {
	return _printf(fmt, t.toString());
}
///@}

/**
 * @brief 样式。
*/
namespace style {
	/**
	 * @brief 格式。
	 * 
	 * 格式可以按位或。
	*/
	enum Format : uint8_t {
		__empty_format  = 1 << 0,
		reset           = 1 << 1,
		bold            = 1 << 2,
		underline       = 1 << 3,
	};

	/**
	 * @brief 颜色。
	 * 
	 * 颜色只能同时使用其中一种。
	 * 
	 * @note 目前只支持前景色。
	*/
	enum Color : uint8_t {
		__empty_color     = 255,
		__reset_color = 0,
		black, white, red, green, yellow, blue, purple, cyan,
	};

	struct Style {
		Style() : f(__empty_format), c(__empty_color)   { }
		Style(Format f, Color c) : f(f), c(c)           { }
		Style(Format f) : f(f), c(__empty_color)        { }
		Style(Color c) : f(__empty_format), c(c)        { }
		Format f;
		Color  c;
	};
	
	template<typename T>
	struct __Wrap {
		__Wrap(Style style, const T& t)
			: style(style), t(t) {}
		const T &t;
		Style style;
		static void _derived_from_wrap();
	};

	namespace predefined {
		/**
		 * @brief 对外暴露的样式的装饰器类。
		*/
		///@{
		#define __ADD_STYLE(_name, _style) \
			template<typename T> \
			struct _name : public style::__Wrap<T> { \
				_name(const T &t) : style::__Wrap<T>(style::_style, t) {} \
			}
			__ADD_STYLE(Black,  black);
			__ADD_STYLE(White,  white);
			__ADD_STYLE(Red,    red);
			__ADD_STYLE(Green,  green);
			__ADD_STYLE(Yellow, yellow);
			__ADD_STYLE(Blue,   blue);
			__ADD_STYLE(Purple, purple);
			__ADD_STYLE(Cyan,   cyan);
			
			__ADD_STYLE(Bold,       bold);
			__ADD_STYLE(Underline,  underline);
		#undef __ADD_STYLE
		///@}
	} // namespace predefined
} // namespace style

/**
 * @brief 样式基类输出。
*/
///@{
int _printf(const char* &fmt, const style::Style &style);
inline int _printf(const char* &fmt, style::Format f) { return _printf(fmt, style::Style(f)); }
inline int _printf(const char* &fmt, style::Color c)  { return _printf(fmt, style::Style(c)); }
///@}

/**
 * @brief 样式包装子类处理。
*/
///@{
template <typename T>
struct _derived_from_wrap {
private:
	template <typename C> static constexpr auto check(C*) -> typename std::is_same<decltype(std::declval<C>()._derived_from_wrap()), void>::type;
	template <typename>   static constexpr std::false_type check(...);
public:
	static constexpr bool value = std::is_same<decltype(check<T>(nullptr)), std::true_type>::value;
};

template <typename T>
typename std::enable_if<_derived_from_wrap<T>::value, int >::type
_printf(const char* &fmt, const T &t) {
	int n = 0;
	n += _skip2percent(fmt);
	const char* _fmt = "";
	n += _printf(_fmt, t.style);
	if(*fmt) --fmt;
	n += _printf(fmt, t.t);
	n += _printf(_fmt, style::reset);
	return n;
}
///@}

/**
 * @brief 内部唯一泛型开展处理。
*/
template<typename F, typename... Args>
int _printf(const char* &fmt, const F& first, Args&&... args) {
	int n = _printf(fmt, first);
	n += _printf(fmt, std::forward<Args&&>(args)...);
	return n;
}

/**
 * @brief 不带参数的 printf 将视 fmt 为 raw 字符串输出。
*/
template<typename = void>
inline int _printf(const char* &fmt) {
	int n = _outputStr(fmt);
	// 不要用 n，输出长度不一定总是等于字符串长度
	while(*fmt) fmt++;
	return n;
}

/**
 * @brief 唯一对外暴露的模板函数。
*/
template<typename... Args>
int printf(const char* fmt, Args&&... args) {
	int n = _printf(fmt, std::forward<Args&&>(args)...);
	if (*fmt) {
		n += _outputStr(fmt);
	}
	return n;
}

} // namespace alts
} // namespace base
} // namespace stuff

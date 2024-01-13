#pragma once

#include <utility>
#include <type_traits>

#include <stdint.h>

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
		#define __ADD_STYLE(_name, _style)                  \
			template<typename T>                            \
			struct _name : public style::__Wrap<T> {        \
				_name(const T &t)                           \
					: style::__Wrap<T>(style::_style, t)    \
				{}                                          \
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

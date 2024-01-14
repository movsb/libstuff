#pragma once

#include <stdint.h>

#include <utility>
#include <type_traits>

/**
 * @brief 标准库容器支持：按需添加。
*/
///@{
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <array>
///@}

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
 * @brief 前向声明（类型定义嵌套）。
 */
///@{
int _printf(const char *&fmt, const std::string &s);
template<typename K, typename V>
int _printf(const char *&fmt, const std::map<K,V> &m);
template<typename K, typename V>
int _printf(const char *&fmt, const std::unordered_map<K,V> &m);
template<typename T>
int _printf(const char *&fmt, const std::vector<T> &v);
template<typename T>
int _printf(const char *&fmt, const std::set<T> &s);
template<typename T>
int _printf(const char *&fmt, const std::unordered_set<T> &s);
template<typename T, std::size_t N>
int _printf(const char *&fmt, const std::array<T,N> &a);
///@}

/**
 * @brief 样式。
// @see https://en.wikipedia.org/wiki/ANSI_escape_code#SGR_(Select_Graphic_Rendition)_parameters
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
		italic          = 1 << 3,
		underline       = 1 << 4,
		blink           = 1 << 5,
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
			__ADD_STYLE(Italic,     italic);
			__ADD_STYLE(Blink,      blink);
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
 * @brief 标准库容器。
*/
///@{
// std::string
inline int _printf(const char *&fmt, const std::string &s) {
	return _printf(fmt, s.c_str());
}

// std::pair
template<typename T1, typename T2>
int _printf(const char *&fmt, const std::pair<T1,T2> &pair) {
	int n = _skip2percent(fmt);
	if (*fmt == 'v') ++fmt;
	else if (!*fmt) {}
	else return n + _unknown(*fmt++);

	{
		const auto &k = pair.first;
		auto _fmt = "%v:";
		n += _printf(_fmt, k);
		n += _outputStr(_fmt);
	}

	{
		const auto &v = pair.second;
		auto _fmt = "%v";
		n += _printf(_fmt, v);
		n += _outputStr(_fmt);
	}

	return n;
}

// std::map & std::unordered_map
// Making a template work with both std::map and std::unordered_map
// https://stackoverflow.com/a/33315998/3628322
// https://stackoverflow.com/a/25750298/3628322
// @todo 支持 Hasher / Allocator 转发。
template<typename Map>
int _printf_map(const char *&fmt, const Map &m) {
	int n = _skip2percent(fmt);
	if (*fmt == 'v') ++fmt;
	else if (!*fmt) {}
	else return n + _unknown(*fmt++);

	n += _outputStr("{");
	
	bool first = true;
	
	for(const auto &pair : m) {
		if (first) first = false;
		else { n += _outputStr(","); }
		n += _printf(fmt, pair);
	}

	n += _outputStr("}");
	return n;
}
template<typename K, typename V>
int _printf(const char *&fmt, const std::map<K,V> &m) {
	return _printf_map(fmt, m);
}
template<typename K, typename V>
int _printf(const char *&fmt, const std::unordered_map<K,V> &m) {
	return _printf_map(fmt, m);
}

// std::vector & std::set & std::unordered_set
template<typename Array>
int _printf_array(const char *&fmt, const Array &a) {
	int n = _skip2percent(fmt);
	if (*fmt == 'v') ++fmt;
	else if (!*fmt) {}
	else return n + _unknown(*fmt++);

	n += _outputStr("[");
	
	bool first = true;
	for(const auto &elem : a) {
		if (first) first = false;
		else { n += _outputStr(","); }
		auto _fmt = "%v";
		n += _printf(_fmt, elem);
		n += _outputStr(_fmt);
	}

	n += _outputStr("]");
	return n;
}
template<typename T>
int _printf(const char *&fmt, const std::vector<T> &v) {
	return _printf_array(fmt, v);
}
template<typename T>
int _printf(const char *&fmt, const std::set<T> &s) {
	return _printf_array(fmt, s);
}
template<typename T>
int _printf(const char *&fmt, const std::unordered_set<T> &s) {
	return _printf_array(fmt, s);
}
template<typename T, std::size_t N>
int _printf(const char *&fmt, const std::array<T,N> &a) {
	return _printf_array(fmt, a);
}
///@}

/**
 * @brief 内部模板。
*/
///{@
template<typename = void>
inline int _printf_t(const char* &fmt) {
	int n = _outputStr(fmt);
	// 不要用 n，输出长度不一定总是等于字符串长度
	while(*fmt) fmt++;
	return n;
}
template<typename F, typename... Args>
int _printf_t(const char* &fmt, const F& first, Args&&... args) {
	int n = _printf(fmt, first);
	n += _printf_t(fmt, std::forward<Args&&>(args)...);
	return n;
}
///@}

/**
 * @brief 唯一对外暴露的模板函数。
*/
template<typename... Args>
int printf(const char* fmt, Args&&... args) {
	int n = _printf_t(fmt, std::forward<Args&&>(args)...);
	return n + _outputStr(fmt);
}

} // namespace alts
} // namespace base
} // namespace stuff

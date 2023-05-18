#pragma once

#include <cstring>
#include <tuple>
#include <charconv>

namespace ebp {
namespace alt {

template<typename T>
std::tuple<T, bool> atoi(const char* str)
{
	T n;
	auto [p, ec] = std::from_chars(str, str + std::strlen(str), n);
	if (ec != std::errc()) {
		// std::cerr << "Error: invalid integer string\n";
		return {T(), false};
	}
	if (p != str + std::strlen(str)) {
		// std::cerr << "Error: invalid characters in integer string\n";
		return {T(), false};
	}
	
	return { std::move(n), true };
}

// 必须成功，否则 abort。
inline auto must(esp_err_t err)
{
	if (err != ESP_OK) { abort(); }
	return std::tuple<>();
}

template <typename T, typename = typename std::enable_if<std::is_pointer<T>::value>::type>
T must(T ptr)
{
	if (ptr == nullptr) { abort(); }
	return ptr;
}


// 多个参数的最后一个必须成功。
template <typename T, typename... Args>
auto must(T&& first, Args&&... args)
{
	return std::tuple_cat(std::tuple<T>(std::forward<T>(first)), must(std::forward<Args>(args)...));
}

// 元组的最后一个元素必须成功。
template<typename... Args>
auto must(std::tuple<Args...>&& args)
{
	return std::apply([](auto&&... args) { return must(std::forward<Args>(args)...); }, args);
}
} // namespace alt
} // namespace ebp

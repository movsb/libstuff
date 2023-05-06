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

// 求数组长度。
template <typename T, std::size_t N>
constexpr std::size_t count_of(T(&)[N]) {
	return N;
}

} // namespace alt
} // namespace ebp

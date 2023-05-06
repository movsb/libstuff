#pragma once

#include <functional>

#include <esp_err.h>

namespace ebp {

namespace helper {

bool _is_last_argument_err_ok(esp_err_t err)
{
	return err == ESP_OK;
}

template<typename T, typename... Args>
bool _is_last_argument_err_ok(T&&, Args&&... args)
{
	return _is_last_argument_err_ok(std::forward<Args>(args)...);
}

template<typename... Args>
auto must(Args&&... args) -> decltype(auto)
{
	if (!_is_last_argument_err_ok(std::forward<Args>(args)...)) {
		abort();
	}
	return std::make_tuple(std::forward<Args>(args)...);
}

} // namespace helper
} // namespace ebp

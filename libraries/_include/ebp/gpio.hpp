#pragma once

#include <functional>

#include "base.hpp"
#include <driver/gpio.h>

namespace ebp {

namespace gpio {

using N             = ::gpio_num_t;

static esp_err_t __installGlobalInterruptHandler(int flag) {
	return ::gpio_install_isr_service(flag);
}

using PullMode      = ::gpio_pull_mode_t;
using Direction     = ::gpio_mode_t;
using InterruptType = ::gpio_int_type_t;

class GPIO {
public:
	explicit GPIO(N n) : _n(n) {
		reset();
	}
	explicit GPIO(uint8_t n) : _n(static_cast<N>(n)) {
		reset();
	}
	
	esp_err_t reset()               { return ::gpio_reset_pin(_n); }

public:
	int         getLevel()          { return ::gpio_get_level(_n); }
	esp_err_t   setLevel(int level) { return ::gpio_set_level(_n, level ? 1 : 0); }

	esp_err_t setPullMode(PullMode mode) {
		return ::gpio_set_pull_mode(_n, static_cast<::gpio_pull_mode_t>(mode));
	}
	esp_err_t setDirection(Direction dir) {
		return ::gpio_set_direction(_n, dir);
	}

	esp_err_t enableInterrupt(InterruptType type, std::function<void()> func) {
		switch (auto err = __installGlobalInterruptHandler(0); err) {
			case ESP_OK:
			case ESP_ERR_INVALID_STATE:
				break;
			default:
				return err;
		}
		if (auto err = ::gpio_set_intr_type(_n, type); err !=ESP_OK) {
			return err;
		}
		_func = func;
		return ::gpio_isr_handler_add(_n, __interruptHandler, this);
	}
	esp_err_t disableInterrupt() {
		// 不管下述函数成功与否始终移除。
		_func = nullptr;
		return ::gpio_isr_handler_remove(_n);
	}

private:
	static void __interruptHandler(void *arg) {
		return reinterpret_cast<GPIO*>(arg)->_func();
	}
	
protected:
	N                       _n;
	std::function<void()>   _func;
};


} // namespace gpio

} // namespace ebp

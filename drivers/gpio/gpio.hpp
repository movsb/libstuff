#pragma once

namespace stuff {
namespace drivers {
namespace gpio{

namespace __abstract {

/**
 * @brief GPIO 抽象定义。
 * @note 传递给实现函数的 t 不是常量，因为实现可能会有状态保存。
*/

template<typename T, typename Config>
class __GPIO {
public:
	template<typename... Args>
	__GPIO(Args&&... args)
		: _t{args...}
		{}
	operator T() { return &_t; }

public:
	void reset() {
		extern void __stuff_reset(T &t);
		__stuff_reset(_t);
	}
	void init(Config config) {
		// 不要用 config Config&
		// 否则初始化列表写起来恶心。
		extern void __stuff_init(T &t, Config &config);
		__stuff_init(_t, config);
	}
	void setValue(bool value) {
		extern void __stuff_set_value(T &t, bool value);
		__stuff_set_value(_t, value);
	}
	void setValue(int value) {
		setValue(value > 0);
	}
	bool getValue() {
		extern bool __stuff_get_value(T &t);
		return __stuff_get_value(_t);
	}
	
protected:
	T _t;
};

} // namespace __abstract
	
}
}
}

#ifdef __STUFF_ESP32__
	#include "gpio_esp32.hpp"
#elif defined(__STUFF_CH32V003__)
	#include "gpio_ch32v003.hpp"
#endif

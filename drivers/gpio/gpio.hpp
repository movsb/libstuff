#pragma once

namespace stuff {
namespace drivers {
namespace gpio{

/**
 * @brief GPIO 的输出方向/工作模式枚举。
*/
struct Direction {
	enum Value {
		Disabled = 0,       // 禁用，
		Input,              // 输入
		Output,             // 输出
		OutputOpenDrain,    // 开漏输出
	};
};

/**
 * @brief 上拉/下拉配置。
 * 
 * 可以按位或。
*/
struct Pull {
	enum Value {
		Up      = 1,
		Down    = 2,
	};
};

template<typename T>
class GPIO {
public:
	template<typename... Args>
	GPIO(Args&&... args)
		: _t{args...}
		{}
	operator T() const { return _t; }

public:
	void reset() {
		extern void __stuff_reset(const T &t);
		__stuff_reset(_t);
	}
	void setDirection(Direction::Value direction) {
		extern void __stuff_set_direction(const T &t, Direction::Value direction);
		__stuff_set_direction(_t, direction);
	}
	void setPull(Pull::Value pull) {
		extern void __stuff_set_pull(const T &t, Pull::Value pull);
		__stuff_set_pull(_t, pull);
	}
	void setValue(bool value) {
		extern void __stuff_set_value(const T &t, bool value);
		__stuff_set_value(_t, value);
	}
	bool getValue() {
		extern bool __stuff_get_value(const T &t);
		return __stuff_get_value(_t);
	}
	
protected:
	T _t;
};

}
}
}

#ifdef __STUFF_ESP32__
	#include "gpio_esp32.hpp"
#endif

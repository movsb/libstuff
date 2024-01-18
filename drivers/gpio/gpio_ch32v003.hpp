#pragma once

#include <stuff/drivers/gpio/gpio.hpp>

#include <ch32v00x/ch32v00x_gpio.h>

namespace stuff {
namespace drivers {
namespace gpio {

class GPIO {
public:
	GPIO(GPIO_TypeDef *group, uint16_t pin)
		: _group(group)
		, _pin(pin)
		, _input(true)
		{}
public:
	void init(GPIOMode_TypeDef mode, GPIOSpeed_TypeDef speed);
	// 用于输入，此时速度无效。
	void init(GPIOMode_TypeDef mode) {
		init(mode, GPIO_Speed_2MHz);
	}
	void setValue(bool value) {
		GPIO_WriteBit(_group, _pin, value ? Bit_SET : Bit_RESET);
	}
	void setValue(int value) {
		return setValue(value > 0);
	}
	bool getValue() {
		auto fn = _input ? GPIO_ReadInputDataBit : GPIO_ReadOutputDataBit;
		return fn(_group, _pin) > 0;
	}
private:
	GPIO_TypeDef    *_group;
	// 暂时只支持单个引脚，
	// 否则 get_value 需要返回一组值。
	// 什么时候会用到？
	uint16_t        _pin;
	bool _input;
};

}
}
}

#pragma once

#include <stuff/drivers/gpio/gpio.hpp>

#include <ch32v00x/ch32v00x_gpio.h>

namespace stuff {
namespace drivers {
namespace gpio {
	struct __Spec {
		GPIO_TypeDef    *group;
		// 暂时只支持单个引脚，
		// 否则 get_value 需要返回一组值。
		// 什么时候会用到？
		uint16_t        pin;
		bool            _input;
		__Spec(GPIO_TypeDef *group, uint16_t pin)
			: group(group), pin(pin), _input(true)
			{}
	};
	struct __Config : public GPIO_InitTypeDef {
		__Config(GPIOSpeed_TypeDef speed, GPIOMode_TypeDef mode) {
			GPIO_Pin    = 0;
			GPIO_Speed 	= speed;
			GPIO_Mode   = mode;
		}
		// 用于输入，此时速度无效。
		__Config(GPIOMode_TypeDef mode)
			: __Config(GPIO_Speed_2MHz, mode)
			{}
	};
	using GPIO = stuff::drivers::gpio::__abstract::__GPIO<__Spec, __Config>;
}
}
}

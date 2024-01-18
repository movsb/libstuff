#include "gpio_ch32v003.hpp"

namespace stuff {
namespace drivers {
namespace gpio {

void GPIO::init(GPIOMode_TypeDef mode, GPIOSpeed_TypeDef speed) {
	// 初始化时钟。
	uint32_t rcc = 0;
	if (_group == GPIOA)        rcc = RCC_APB2Periph_GPIOA;
	else if(_group == GPIOC)    rcc = RCC_APB2Periph_GPIOC;
	else if(_group == GPIOD)    rcc = RCC_APB2Periph_GPIOD;
	else {
		// TODO warning unused
	}
	RCC_APB2PeriphClockCmd(rcc, ENABLE);

	auto config = GPIO_InitTypeDef {
		.GPIO_Pin   = _pin,
		.GPIO_Speed = speed,
		.GPIO_Mode  = mode,
	};
	GPIO_Init(_group, &config);

	switch (config.GPIO_Mode) {
	case GPIO_Mode_AIN:
	case GPIO_Mode_IN_FLOATING:
	case GPIO_Mode_IPD:
	case GPIO_Mode_IPU:
		_input = true;
		break;
	default:
		_input = false;
		break;
	}
}

} // namespace gpio
} // namespace drivers
} // namespace stuff

#include "gpio_ch32v003.hpp"

namespace stuff {
namespace drivers {
namespace gpio {
namespace __abstract {

void __stuff_reset(__Spec & /*spec*/) {

}

void __stuff_init(__Spec &spec, __Config &config) {
	// 初始化时钟。
	uint32_t rcc = 0;
	if (spec.group == GPIOA)        rcc = RCC_APB2Periph_GPIOA;
	else if(spec.group == GPIOC)    rcc = RCC_APB2Periph_GPIOC;
	else if(spec.group == GPIOD)    rcc = RCC_APB2Periph_GPIOD;
	else {
		// TODO warning unused
	}
	RCC_APB2PeriphClockCmd(rcc, ENABLE);
	
	auto raw = static_cast<GPIO_InitTypeDef&>(config);
	raw.GPIO_Pin = spec.pin;
	GPIO_Init(spec.group, &raw);

	switch (config.GPIO_Mode) {
	case GPIO_Mode_AIN:
	case GPIO_Mode_IN_FLOATING:
	case GPIO_Mode_IPD:
	case GPIO_Mode_IPU:
		spec._input = true;
		break;
	default:
		spec._input = false;
		break;
	}
}

/**
 * @todo 如果是输入，设置有意义吗？
*/
void __stuff_set_value(__Spec &spec, bool value) {
	GPIO_WriteBit(spec.group, spec.pin, value ? Bit_SET : Bit_RESET);
}

bool __stuff_get_value(__Spec &spec) {
	auto fn = spec._input ? GPIO_ReadInputDataBit : GPIO_ReadOutputDataBit;
	return fn(spec.group, spec.pin) > 0;
}

} // namespace __abstract

} // namespace gpio
} // namespace drivers
} // namespace stuff

#include <stuff/drivers/gpio/gpio.hpp>

#include <driver/gpio.h>

namespace stuff {
namespace drivers {
namespace gpio {
namespace __abstract {

void __stuff_reset(const gpio_num_t &num) {

}

void __stuff_set_direction(const gpio_num_t &num, Direction::Value direction) {
	gpio_mode_t mode;
	switch (direction) {
	case Direction::Input:
		mode = GPIO_MODE_INPUT;
		break;
	case Direction::Output:
		mode = GPIO_MODE_OUTPUT;
		break;
	case Direction::OutputOpenDrain:
		mode = GPIO_MODE_OUTPUT_OD;
		break;
	default:
		mode = GPIO_MODE_DISABLE;
		break;
	}
	::gpio_set_direction(num, mode);
}

void __stuff_set_pull(const gpio_num_t &num, Pull::Value pull) {
	pull & Pull::Up ? ::gpio_pullup_en(num) : ::gpio_pullup_dis(num);
	pull & Pull::Down ? ::gpio_pulldown_en(num) : ::gpio_pulldown_dis(num);
}

void __stuff_set_value(const gpio_num_t &num, bool value) {
	::gpio_set_level(num, value ? 1 : 0);
}

bool __stuff_get_value(const gpio_num_t &num) {
	return ::gpio_get_level(num) > 0;
}

} // namespace __abstract

} // namespace gpio
} // namespace drivers
} // namespace stuff

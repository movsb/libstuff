#include <stuff/drivers/gpio/gpio.hpp>

#include <driver/gpio.h>

namespace stuff {
namespace drivers {
namespace gpio {
	using GPIO = stuff::drivers::gpio::__abstract::GPIO<gpio_num_t>;
}
}
}

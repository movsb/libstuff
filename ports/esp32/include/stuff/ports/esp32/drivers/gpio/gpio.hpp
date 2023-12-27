#include <stuff/drivers/gpio/gpio.hpp>
#include <driver/gpio.h>

namespace stuff {
namespace ports {
namespace esp8266 {
namespace drivers {
namespace gpio {
	using GPIO = stuff::drivers::gpio::GPIO<gpio_num_t>;
}
}
}
}
}

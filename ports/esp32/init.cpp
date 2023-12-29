#include <stuff/ports/esp32/init.hpp>

extern void _init_time();

namespace stuff {
namespace ports {
namespace esp32 {

void init() {
	_init_time();
}

}
}
}

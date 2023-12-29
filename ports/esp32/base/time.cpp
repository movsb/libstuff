#include <stdint.h>
#include <inttypes.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace stuff {
namespace base {
namespace time {

void __stuff_sleep_us(int64_t microseconds) {
	uint32_t ticks = microseconds / 1000 / portTICK_PERIOD_MS;
	vTaskDelay(ticks);
}

} // namespace time
} // namespace base
} // namespace stuff

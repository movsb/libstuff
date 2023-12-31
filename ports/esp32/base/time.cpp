#include <stdint.h>
#include <inttypes.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_timer.h>

namespace stuff {
namespace base {
namespace time {

void __stuff_sleep_us(int64_t microseconds) {
	uint32_t ticks = microseconds / 1000 / portTICK_PERIOD_MS;
	vTaskDelay(ticks);
}

int64_t __stuff_get_uptime() {
	return ::esp_timer_get_time();
}

} // namespace time
} // namespace base
} // namespace stuff

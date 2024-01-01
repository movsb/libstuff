#include <stdint.h>
#include <inttypes.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_timer.h>

namespace stuff {
namespace base {
namespace time {

void __stuff_sleep_us(int64_t microseconds) {
	int64_t ticks = microseconds / 1000 / portTICK_PERIOD_MS;
	static_assert(sizeof(int64_t) >= sizeof(TickType_t));
	if (ticks > static_cast<int64_t>(portMAX_DELAY)) {
		ticks = portMAX_DELAY;
	}
	vTaskDelay(static_cast<TickType_t>(ticks));
}

int64_t __stuff_get_uptime() {
	return ::esp_timer_get_time();
}

} // namespace time
} // namespace base
} // namespace stuff

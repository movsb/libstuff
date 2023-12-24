#include <stdint.h>
#include <inttypes.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

extern "C" void __stuff_base_time_sleep_us(int64_t microseconds) {
	uint32_t ticks = microseconds / 1000 / portTICK_PERIOD_MS;
	vTaskDelay(ticks);
}

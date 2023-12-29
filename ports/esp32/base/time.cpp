#include <stdint.h>
#include <inttypes.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <stuff/base/time/duration.hpp>

static void sleep_us(int64_t microseconds) {
	uint32_t ticks = microseconds / 1000 / portTICK_PERIOD_MS;
	vTaskDelay(ticks);
}

void _init_time() {
	stuff::base::time::set_sleep_func(sleep_us);
}

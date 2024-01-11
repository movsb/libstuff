#include <stdint.h>
#include <inttypes.h>

#include <ch32v00x/debug.h>
#include <ch32v00x/system_ch32v00x.h>

namespace stuff {
namespace base {
namespace time {

void __stuff_sleep_us(int64_t microseconds) {
	// TODO 可能溢出？
	Delay_Us(static_cast<uint32_t>(microseconds));
}

int64_t __stuff_get_uptime() {
	return SysTick_GetUptime();
}

} // namespace time
} // namespace base
} // namespace stuff

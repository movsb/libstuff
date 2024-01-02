#include <stdint.h>
#include <stuff/base/time/duration.hpp>

namespace stuff {
namespace base {
namespace time {

/**
 * @brief 外部实现的睡眠函数。
 * 
 * @param microseconds 需要睡眠的微秒数。
*/
extern void __stuff_sleep_us(int64_t microseconds);

void sleep(const Duration &duration) {
	__stuff_sleep_us(duration.microseconds());
}

void after(const Duration &duration, std::function<void()> callback) {
	sleep(duration);
	callback();
}

void tick(const Duration &duration, std::function<void()> callback) {
	while((1)) {
		sleep(duration);
		callback();
	}
}

} // namespace time
} // namespace base
} // namespace stuff

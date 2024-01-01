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

} // namespace time
} // namespace base
} // namespace stuff

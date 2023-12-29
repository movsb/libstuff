#include <stuff/base/time/duration.hpp>


namespace stuff {
namespace base {
namespace time {

#if __STUFF_HAS_SLEEP__
// 外部实现的睡眠函数。
void (*__stuff_sleep_us)(int64_t microseconds);

void set_sleep_func(void (*sleep_func)(int64_t microseconds)) {
	__stuff_sleep_us = sleep_func;
}

void sleep(const Duration &duration) {
	__stuff_sleep_us(duration.microseconds());
}
#endif

} // namespace time
} // namespace base
} // namespace stuff

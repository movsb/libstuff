#include <stdint.h>
#include <stuff/base/time/duration.hpp>

namespace stuff {
namespace base {
namespace time {

extern void __stuff_sleep_us(int64_t microseconds);

void sleep(const Duration &duration) {
	__stuff_sleep_us(duration.microseconds());
}

} // namespace time
} // namespace base
} // namespace stuff

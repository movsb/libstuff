#include <stdint.h>
#include <stuff/base/time/duration.hpp>

namespace stuff {
namespace base {
namespace time {

#if __STUFF_HAS_SLEEP__
extern void __stuff_sleep_us(int64_t microseconds);

void sleep(const Duration &duration) {
	__stuff_sleep_us(duration.microseconds());
}
#endif

} // namespace time
} // namespace base
} // namespace stuff

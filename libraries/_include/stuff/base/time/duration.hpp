#pragma once

#include <stdint.h>
#include <inttypes.h>
#include <assert.h>
#include <chrono>

namespace stuff {
namespace base {
namespace time {

/**
 * @brief 持续时间，即时长。
 * 
 * 内部暂定用 int64_t 表示，以纳秒为单位。
 * 2^63 nanoseconds to years is 292.2770246269 years
 * 
 * @todo 如何要考虑空间占用，可以用 2^31 表示毫秒，
 *       大概有 2^31 ms to days is 24.8551348148 days
*/
class Duration {
public:
	Duration(int64_t nanoseconds) : _t(nanoseconds) {}
	
	/**
	 * @brief 支持 1s, 1ms 1.1s 这类的带单位的表示法初始化，简化书写。
	 * 
	 * @note 需要 using namespace time::literals 开启。
	 * @note 会增加 0.1KB 的空间使用。编译期常量，不会增加时间开销。
	 */
	template<typename Rep, typename Period>
	Duration(const std::chrono::duration<Rep, Period> &duration) :
		_t(std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count()) {}

	int64_t nanoseconds()   const { return _t;                          }
	int64_t microseconds()  const { return nanoseconds()    / 1000;     }
	int64_t milliseconds()  const { return microseconds()   / 1000;     }
	double  seconds()       const { return milliseconds()   / 1000.0;   }
	double  minutes()       const { return seconds()        / 60.0;     }
	double  hours()         const { return minutes()        / 60.0;     }
	double  days()          const { return hours()          / 24.0;     }

#if __STUFF_HAS_OS__
	uint32_t osTicks()        const {
		auto n = __stuff_base_time_ticks_of(milliseconds());
		return n < 0 ? 0 : n;
	}
#endif
	
	Duration operator*(double n)                const { return _t * n;              }
	Duration operator/(double n)                const { return _t / n;              }
	bool operator<(const Duration& other)       const { return _t < other._t ;      }
	bool operator>(const Duration& other)       const { return _t > other._t ;      }
	bool operator==(const Duration& other)      const { return _t == other._t;      }
	bool operator<=(const Duration& other)      const { return ! (*this > other);   }
	bool operator>=(const Duration& other)      const { return ! (*this < other);   }
	bool operator!=(const Duration& other)      const { return ! (*this == other);  }

protected:
	int64_t _t;
};

// To Allow expressions like N * Duration, where N is a constant.
inline Duration operator*(double n, const Duration &duration) { return duration * n; }
inline Duration operator/(double n, const Duration &duration) { return duration / n; }

inline Duration nanoseconds (int64_t n) { return Duration       (n * 1);    }
inline Duration microseconds(int64_t n) { return nanoseconds    (n * 1000); }
inline Duration milliseconds(int64_t n) { return microseconds   (n * 1000); }
inline Duration seconds     (int64_t n) { return milliseconds   (n * 1000); }
inline Duration minutes     (int64_t n) { return seconds        (n * 60);   }
inline Duration hours       (int64_t n) { return minutes        (n * 60);   }
inline Duration days        (int64_t n) { return hours          (n * 24);   }

// 使用 double 会增加 2KB 的 .text（软件浮点运算情况下）。
// 当然，除非完全不使用 double 表达式，否则是非常难以避免的，
// 所以我提供了 int64_t 的版本。即省空间又省时间，效率提升。
inline Duration seconds     (double n)  { return milliseconds   (n * 1000); }
inline Duration minutes     (double n)  { return seconds        (n * 60);   }
inline Duration hours       (double n)  { return minutes        (n * 60);   }
inline Duration days        (double n)  { return hours          (n * 24);   }

// 一些常用的别名
inline Duration ns          (int64_t n) { return nanoseconds(n);    }
inline Duration us          (int64_t n) { return microseconds(n);   }
inline Duration ms          (int64_t n) { return milliseconds(n);   }

// 支持 1s, 1ms 这类的带单位的表示法。
namespace literals {
	using namespace std::literals::chrono_literals;
}

#if !defined(__STUFF_HAS_SLEEP__)
	#define __STUFF_HAS_SLEEP__ 1
#endif

#if __STUFF_HAS_SLEEP__
void sleep(const Duration &duration);
#endif

void set_sleep_func(void (*sleep_func)(int64_t microseconds));

} // namespace time
} // namespace base
} // namespace stuff

#pragma once

#include <stdint.h>
#include <inttypes.h>
#include <assert.h>

extern "C" {

void    __stuff_base_time_sleep_us(int64_t microseconds);

}

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
inline Duration seconds     (double n)  { return milliseconds   (n * 1000); }
inline Duration minutes     (double n)  { return seconds        (n * 60);   }
inline Duration hours       (double n)  { return minutes        (n * 60);   }
inline Duration days        (double n)  { return hours          (n * 24);   }

// 别名
inline Duration ns          (int64_t n) { return nanoseconds(n);    }
inline Duration us          (int64_t n) { return microseconds(n);   }
inline Duration ms          (int64_t n) { return milliseconds(n);   }

inline void sleep(Duration duration) {
	return __stuff_base_time_sleep_us(duration.microseconds());
}

} // namespace time
} // namespace base
} // namespace stuff

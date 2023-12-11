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

inline const Duration
	Nanosecond      = { 1 },
	Microsecond     = { Nanosecond    * 1000 },
	Millisecond     = { Microsecond   * 1000 },
	Second          = { Millisecond   * 1000 },
	Minute          = { Second        * 60   },
	Hour            = { Minute        * 60   },
	Day             = { Hour          * 24   };


inline void sleep(Duration duration) {
	return __stuff_base_time_sleep_us(duration.microseconds());
}

void test();

} // namespace time
} // namespace base
} // namespace stuff

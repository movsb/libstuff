#pragma once

#include <stdint.h>
#include <inttypes.h>
#include <assert.h>

#if __STUFF_HAS_CALENDAR__ || __STUFF_HAS_UPTIME__
#include <string>
#endif

extern "C" {
#if __STUFF_HAS_OS__
	int32_t __stuff_base_time_os_ticks_of(int64_t milliseconds);
#endif

void    __stuff_base_time_sleep_us(int64_t microseconds);
	
#if __STUFF_HAS_UPTIME__
	int64_t __stuff_base_time_get_uptime();
#endif

#if __STUFF_HAS_TZ__
	// from glibc.
	int  setenv(const char *__string, const char *__value, int __overwrite);
	void tzset(void);
#endif

#if __STUFF_HAS_CALENDAR__
	#if !__STUFF_USE_GET_TIME_OF_DAY__
		void __stuff_get_calendar(int64_t *seconds, int64_t *microseconds);
	#endif
#endif
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

// protected:
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

#if __STUFF_HAS_CALENDAR__ || __STUFF_HAS_UPTIME__

enum class Month : uint8_t {
	January     = 1,
	February    = 2,
	March       = 3,
	April       = 4,
	May         = 5,
	June        = 6,
	July        = 7,
	August      = 8,
	September   = 9,
	October     = 10,
	November    = 11,
	December    = 12,
};

enum class Weekday : uint8_t {
	Sunday      = 0,
	Monday      = 1,
	Tuesday     = 2,
	Wednesday   = 3,
	Thursday    = 4,
	Friday      = 5,
	Saturday    = 6,
};

class Time {
public:
	Time() {
		_sec    = 0;
		_micro  = 0;
		_mono   = 0;
		_offset = 0;
	}

private:
	int64_t     _sec        :33;    // s, 272 years, from 1970-01-01 00:00:00.
	int64_t     _micro      :20;    // us, 0-999999us
	int64_t     _mono       :50;    // us, 35 years
	int64_t     _offset     :7;     // 15m, the number 15 minutes east from UTC, ± 14 hours.

public:
	bool empty() const { return _sec == 0 && _micro == 0 && _mono == 0; }
	Time operator+(const Duration &d) const;
	Duration operator-(const Time& other) const {
		assert(hasMono() && other.hasMono());
		return (_mono - other._mono) * Microsecond;
	}
	bool operator>(const Time &other) const {
		if (hasMono() && other.hasMono()) { return _mono > other._mono; }
		return (_sec > other._sec) || (_sec == other._sec && _micro > other._micro);
	}
	bool operator<(const Time &other) const {
		if (hasMono() && other.hasMono()) { return _mono < other._mono; }
		return (_sec < other._sec) || (_sec == other._sec && _micro < other._micro);
	}
	bool operator==(const Time &other) const {
		if (hasMono() && other.hasMono()) { return _mono == other._mono; }
		return _sec == other._sec && _micro == other._micro;
	}
	
	std::tuple<int, Month, int> date() const {
		auto [year, month, day, _] = absDate(abs());
		return { year, month, day };
	}

	int     year()          const { return std::get<0>(absDate(abs())); }
	Month   month()         const { return std::get<1>(absDate(abs())); }
	int     day()           const { return std::get<2>(absDate(abs())); }
	int     yearDay()       const { return std::get<3>(absDate(abs())); }
	int     hour()          const { return int(abs() % (24*60*60)) / (60*60);   }
	int     minute()        const { return int(abs() % (60*60)) / (60);         }
	int     second()        const { return int(abs() % 60);                     }
	int     microsecond()   const { return _micro; }
	int     nanosecond()    const { return 0;      }
	
	std::string toString() const {
		auto t = _sec * 1000000 + _micro;
		char buf[64];
		snprintf(&buf[0], sizeof(buf)/sizeof(buf[0]), "%lld", t);
		return std::string(buf);
	}
	
private:
	friend Time now();
	friend Time ticks();

	bool hasMono() const { return _mono > 0; }
	void stripMono() { _mono = 0; }
	// abs returns the time t as an absolute time, adjusted by the zone offset.
	// It is called when computing a presentation property like Month or Hour.
	uint64_t abs() const { return _sec + _offset * 15 * 60; }
	// int *year, Month *month, int *day, int *yday
	std::tuple<int, Month, int, int> absDate(uint64_t abs) const;
};

#if __STUFF_HAS_CALENDAR__
Time now();
#endif

#if __STUFF_HAS_UPTIME__
Time ticks();
#endif

#if __STUFF_HAS_TZ__
void setTZ(int offset);
#endif

inline Duration since(const Time& past)    { return now() - past;      }
inline Duration until(const Time& future)  { return future - now();    }

#endif // __STUFF_HAS_CALENDAR__ || __STUFF_HAS_UPTIME__

} // namespace time
} // namespace base
} // namespace stuff

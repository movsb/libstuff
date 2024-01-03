#pragma once

#include <stdint.h>
#include <inttypes.h>
#include <assert.h>
#include <string>
#include <tuple>

#include "duration.hpp"

namespace stuff {
namespace base {
namespace time {

#if defined(__STUFF_HAS_UPTIME__)
	extern int64_t __stuff_get_uptime();
#endif

#if defined(__STUFF_HAS_TZ__)
extern "C" {
	// from glibc.
	int  setenv(const char *__string, const char *__value, int __overwrite);
	void tzset(void);
}
#endif

#if defined(__STUFF_HAS_CALENDAR__)
	#if !__STUFF_USE_GET_TIME_OF_DAY__
		void __stuff_get_calendar(int64_t *seconds, int64_t *microseconds);
	#endif
#endif

#if defined(__STUFF_HAS_CALENDAR__) || defined(__STUFF_HAS_UPTIME__)

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
		reset();
	}

private:
	int64_t     _sec        :33;    // s, 272 years, from 1970-01-01 00:00:00.
	int64_t     _micro      :20;    // us, 0-999999us
	int64_t     _mono       :50;    // us, 35 years
	int64_t     _offset     :7;     // 15m, the number 15 minutes east from UTC, ± 14 hours.

public:
	void reset() {
		_sec    = 0;
		_micro  = 0;
		_mono   = 0;
		_offset = 0;
	}
	bool zero() const { return _sec == 0 && _micro == 0 && _mono == 0; }
	Time operator+(const Duration &d) const;
	Duration operator-(const Time& other) const {
		assert(hasMono() && other.hasMono());
		return microseconds(_mono - other._mono);
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

	int     days()          const { return _sec / 60 / 60/ 24; }
	
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

#if defined(__STUFF_HAS_CALENDAR__)
Time now();
#endif

#if defined(__STUFF_HAS_UPTIME__)
Time ticks();
#endif

#if defined(__STUFF_HAS_TZ__)
void setTZ(int offset);
#endif

inline Duration since(const Time& past)    { return now() - past;      }
inline Duration until(const Time& future)  { return future - now();    }

#endif // __STUFF_HAS_CALENDAR__ || __STUFF_HAS_UPTIME__

} // namespace time
} // namespace base
} // namespace stuff

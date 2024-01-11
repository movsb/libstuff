#include <stuff/base/time/time.hpp>

#if defined(__STUFF_USE_GET_TIME_OF_DAY__)
	#include <sys/time.h>
#endif

namespace stuff {
namespace base {
namespace time {

#if defined(__STUFF_HAS_CALENDAR__) || defined(__STUFF_HAS_UPTIME__)

// 怎么感觉实现得好复杂。这不比 int64_t + int64_t 复杂多了？
Time  Time::operator+(const Duration &d) const {
	Time t(*this);
	
	int64_t dSec = d.microseconds() / int64_t(1e6);
	int64_t mSec = t._micro + d.microseconds() % int64_t(1e6);
	
	if (mSec >= 1e6) {
		dSec++;
		mSec -= 1e6;
	} else if (mSec < 0) {
		dSec--;
		mSec += 1e6;
	}

	t._micro = mSec;
	t._sec += dSec;

	t._mono += d.microseconds();

	return t;
}

namespace {
	constexpr int secondsPerMinute  = 60;
	constexpr int secondsPerHour    = 60*60;
	constexpr int secondsPerDay     = 24*60*60;
	constexpr int daysPerWeek       = 7 * secondsPerDay;
	constexpr int daysPer400Years   = 365 * 400 + 97;
	constexpr int daysPer100Years   = 365 * 100 + 24;
	constexpr int daysPer4Years     = 365 * 4   +  1;

	static bool isLeapYear(int year) {
		return (year % 4 == 0) && (year % 100 != 0 || year % 400 == 0);
	}


	// daysBefore[m] counts the number of days in a non-leap year
	// before month m begins. There is an entry for m=12, counting
	// the number of days before January of next year (365).
	uint32_t daysBefore[] = {
		0,
		31,
		31 + 28,
		31 + 28 + 31,
		31 + 28 + 31 + 30,
		31 + 28 + 31 + 30 + 31,
		31 + 28 + 31 + 30 + 31 + 30,
		31 + 28 + 31 + 30 + 31 + 30 + 31,
		31 + 28 + 31 + 30 + 31 + 30 + 31 + 31,
		31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30,
		31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31,
		31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30,
		31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30 + 31,
	};
}

std::tuple<int, Month, int, int> Time::absDate(uint64_t absolute) const
{
	// Split into time and day.
	uint64_t d = absolute / secondsPerDay;
	
	// Account for 400 year cycles.
	uint64_t n = d / daysPer400Years;
	uint64_t y = 400 * n;
	d -= daysPer400Years * n;

	// Cut off 100-year cycles.
	// The last cycle has one extra leap year, so on the last day
	// of that year, day / daysPer100Years will be 4 instead of 3.
	// Cut it back down to 3 by subtracting n>>2.
	n = d / daysPer100Years;
	n -= n >> 2;
	y += 100 * n;
	d -= daysPer100Years * n;

	// Cut off 4-year cycles.
	// The last cycle has a missing leap year, which does not
	// affect the computation.
	n = d / daysPer4Years;
	y += 4 * n;
	d -= daysPer4Years * n;
	
	// Cut off years within a 4-year cycle.
	// The last year is a leap year, so on the last day of that year,
	// day / 365 will be 4 instead of 3. Cut it back down to 3
	// by subtracting n>>2.
	n = d / 365;
	n -= n >> 2;
	y += n;
	d -= 365 * n;

	int year = int(int64_t(y) + 1970);
	int yday = int(d);
	
	Month month;
	
	int day = int(d);
	if (isLeapYear(year)) {
		if (day > 31 + 29 - 1) {
			day--;
		} else if (day == 31 + 29 -1) {
			month = Month::February;
			day = 29;
			return { year, month, day, yday };
		}
	}
	
	month = Month(day / 31);
	int end = int(daysBefore[int(month)+1]);
	int begin;
	if (day >= end) {
		++*reinterpret_cast<std::underlying_type<Month>::type*>(&month);
		begin = end;
	} else {
		begin = int(daysBefore[int(month)]);
	}
	
	++*reinterpret_cast<std::underlying_type<Month>::type*>(&month);
	day = day - begin + 1;
	
	return { year, month, day, yday };
}

#if __STUFF_HAS_TZ__

static int g_timezone_offset =  0;

// offset is in seconds east from the UTC.
//
// https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32c3/api-reference/system/system_time.html#id3
// https://www.gnu.org/software/libc/manual/html_node/TZ-Variable.html
// std offset
// The std string specifies the name of the time zone.
// The offset specifies the time value you must add to the local time to get a Coordinated Universal Time value.
// GMT+8 = CST, CST-8 = GMT.
void setTZ(int offset)
{
	assert(-14*60*60 <= offset && offset <= 14*60*60);

	g_timezone_offset = offset;
	offset = -offset;

	char    sign    = offset > 0 ? '+' : '-';
	uint8_t hours   = offset / 60 / 60;
	uint8_t minutes = offset / 60 % 60;

	// CST-08:00 (CST is not care)
	char buf[sizeof("CST-08:00")];

	// the reminder operations is to eliminate warnings from gcc.
	snprintf(buf, sizeof(buf), "CST%c%02d:%02d", sign, hours % 24, minutes % 60);
	setenv("TZ", buf, 1);
	tzset();
}

#endif

#if defined(__STUFF_HAS_CALENDAR__) && defined(__STUFF_USE_GET_TIME_OF_DAY__)
void __stuff_get_calendar(int64_t *seconds, int64_t *microseconds) {
	timeval val{0, 0};
	if (gettimeofday(&val, NULL) != 0) {
		abort();
	}
	*seconds = val.tv_sec;
	*microseconds = val.tv_usec;
}
#endif

Time now()
{
	
	Time t = {};

#if defined(__STUFF_HAS_CALENDAR__)
	{
		int64_t s, us;
		__stuff_get_calendar(&s, &us);
		t._sec = s;
		t._micro = us;
	}
#endif

#if defined(__STUFF_HAS_UPTIME__)
	t._mono = __stuff_get_uptime();
#endif

#if defined(__STUFF_HAS_TZ__)
	t._offset   = g_timezone_offset / 60 / 15;
#endif

	return t;
}

#if defined(__STUFF_HAS_UPTIME__)
/**
 * @brief 返回自系统运行以来的微秒数。
 * 
 * 主要用来计数、统计等。
 * 
 * @note 日历和时区等信息不可用。
*/
Time ticks()
{
	Time t;
	t._mono = __stuff_get_uptime();
	return t;
}
#endif

#endif //__STUFF_HAS_CALENDAR__ || __STUFF_HAS_UPTIME__

} // namespace time
} // namespace base
} // namespace stuff

#pragma once

#include <string>
#include <sys/time.h>

#include "base.hpp"

#include "esp_etm.h"
#include "esp_timer.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace ebp {

namespace time {
	
class Duration {
public:
	Duration(int64_t nanoseconds) : _t(nanoseconds) {}

	int64_t nanoseconds()   const { return _t;                      }
	int64_t microseconds()  const { return nanoseconds()    / 1000; }
	int64_t milliseconds()  const { return microseconds()   / 1000; }
	int64_t seconds()       const { return milliseconds()   / 1000; }
	int64_t minutes()       const { return seconds()        / 60;   }
	int64_t hours()         const { return minutes()        / 60;   }
	int64_t days()          const { return hours()          / 24;   }
	
	Duration operator*(int64_t n)       const { return _t * n;          }
	Duration operator/(int64_t n)       const { return _t / n;          }
	
protected:
	int64_t _t;
};
	
Duration operator*(int64_t n, const Duration &duration) { return duration * n; }
Duration operator/(int64_t n, const Duration &duration) { return duration / n; }
	
static const Duration
	Nanosecond      = { 1 },
	Microsecond     = { Nanosecond    * 1000 },
	Millisecond     = { Microsecond   * 1000 },
	Second          = { Millisecond   * 1000 },
	Minute          = { Second        * 60   },
	Hour            = { Minute        * 60   },
	Day             = { Hour          * 24   };


static void sleep(Duration duration) {
	vTaskDelay(duration.milliseconds() / portTICK_PERIOD_MS);
}

class Time {
private:
	int64_t     _sec        :33;    // s, 272 years, from 1970-01-01 00:00:00.
	int64_t     _micro      :20;    // us, 0-999999us
	int64_t     _mono       :50;    // us, 35 years
	int64_t     _offset     :7;     // 15m, the number 15 minutes, ± 14 hours.

public:
	Duration operator-(const Time& other) {
		assert(_mono > 0 && other._mono > 0);
		return (_mono - other._mono) * Microsecond;
	}
	
	std::string toString() const {
		auto t = _sec * 1000000 + _micro;
		char buf[64];
		snprintf(&buf[0], sizeof(buf)/sizeof(buf[0]), "%lld", t);
		return std::string(buf);
	}
	
private:
	friend Time now();
};

Time now() {
	timeval val{0, 0};
	if (gettimeofday(&val, NULL) != 0) {
		// panic
	}
	
	Time t = {};
	t._sec      = val.tv_sec;
	t._micro    = val.tv_usec;
	t._offset   = 0;
	t._mono     = esp_timer_get_time();
	return t;
}

} // namespace time

} // namespace ebp

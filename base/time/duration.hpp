#pragma once

#include <stdint.h>
#include <inttypes.h>
#include <assert.h>
#include <chrono>
#include <limits>
#include <functional>

namespace stuff {
namespace base {
namespace time {

/**
 * @brief 持续时间，即时长。同时也用于表示两个时间值的间隔，可正可负。
 * 
 * 内部暂定用 int64_t 表示，以微秒为单位。同时可以避免在不支持除法指令的 CPU 上
 * 生成 __divdi3 方法（软件除法），会占用若干 KB 的空间。不划算。
 * 
 * 2^63 nanoseconds  to years is 292.2770246269     years
 * 2^63 microseconds to years is 292,277.0246269277 years
 * 
 * @todo 如何要考虑空间占用，可以用 2^31 表示毫秒，
 *       大概有 2^31 ms to days is 24.8551348148 days
 *       不是很大，长期任务容易溢出。
*/
class Duration {
public:
	/**
	 * @todo 私有化。
	*/
#ifdef __STUFF_DURATION_NANO__
	explicit Duration(int64_t nanoseconds)  : _t(nanoseconds) {}
#else
	explicit Duration(int64_t microseconds) : _t(microseconds) {}
#endif
	
	/**
	 * @brief 支持 1s, 1ms 1.1s 这类的带单位的表示法初始化，简化书写。
	 * 
	 * @note 需要 using namespace time::literals 开启。
	 * @note 会增加 0.1KB 的空间使用。编译期常量，不会增加时间开销。
	 */
	template<typename Rep, typename Period>
	Duration(const std::chrono::duration<Rep, Period> &duration) :
		_t(std::chrono::duration_cast<
#ifdef __STUFF_DURATION_NANO__
			std::chrono::nanoseconds
#else
			std::chrono::microseconds
#endif
		>(duration).count()) {}

#ifdef __STUFF_DURATION_NANO__
	int64_t nanoseconds()   const { return _t;                          }
	int64_t microseconds()  const { return nanoseconds()    / 1000;     }
#else
	int64_t microseconds()  const { return _t;                          }
#endif
	int64_t milliseconds()  const { return microseconds()   / 1000;     }
	double  seconds()       const { return milliseconds()   / 1000.0;   }
	double  minutes()       const { return seconds()        / 60.0;     }
	double  hours()         const { return minutes()        / 60.0;     }
	double  days()          const { return hours()          / 24.0;     }

#if __STUFF_HAS_OS__
	uint32_t osTicks()        const {
		extern int64_t __stuff_ticks_of(int64_t microseconds);
		auto n = __stuff_ticks_of(milliseconds());
		// TODO 有精度丢失吗？
		return n < 0 ? 0 : static_cast<uint32_t>(n);
	}
#endif
	
	Duration operator*(double n)                const { return Duration(static_cast<int64_t>(_t * n));  }
	Duration operator/(double n)                const { return Duration(static_cast<int64_t>(_t / n));  }
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

#ifdef __STUFF_DURATION_NANO__
	inline Duration nanoseconds (int64_t n) { return Duration       (n * 1);    }
	inline Duration microseconds(int64_t n) { return nanoseconds    (n * 1000); }
#else
	inline Duration microseconds(int64_t n) { return Duration(n);               }
#endif
	inline Duration milliseconds(int64_t n) { return microseconds   (n * 1000); }

// 使用 double 会增加 2KB 的 .text（软件浮点运算情况下）。
// 当然，除非完全不使用 double 表达式，否则是非常难以避免的，
// 所以我提供了 int64_t 的版本。即省空间又省时间，效率提升。
inline Duration seconds     (double n)  { return milliseconds   (n * 1000); }
inline Duration minutes     (double n)  { return seconds        (n * 60);   }
inline Duration hours       (double n)  { return minutes        (n * 60);   }
inline Duration days        (double n)  { return hours          (n * 24);   }

// 一些常用的别名
#ifdef __STUFF_DURATION_NANO__
inline Duration ns          (int64_t n) { return nanoseconds(n);    }
#endif
inline Duration us          (int64_t n) { return microseconds(n);   }
inline Duration ms          (int64_t n) { return milliseconds(n);   }

// 支持 1s, 1ms 这类的带单位的表示法。
namespace literals {
	using namespace std::literals::chrono_literals;
}

/**
 * @brief 在当前任务中睡眠指定的时长。
 * 
 * @note \p forever 可以永久睡眠。
 * @note 永久是多久？根据实现定义。
*/
void sleep(const Duration &duration);

const Duration forever = Duration(std::numeric_limits<int64_t>::max());

/**
 * @brief 在指定的时长后调用回调函数一次。
 * @note 回调函数会在指定的时间到达后在当前任务中执行。
 *          如果不希望如此，可以考虑 timer::after。
*/
void after(const Duration &duration, std::function<void()> callback);

/**
 * @brief 周期地调用指定的函数。
 * @note 回调函数会在指定的时间到达后在当前任务中执行。
 *          如果不希望如此，可以考虑 timer::tick。
*/
void tick(const Duration &duration, std::function<void()> callback);
} // namespace time
} // namespace base
} // namespace stuff

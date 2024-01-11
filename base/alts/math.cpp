#include <stdbool.h>
#include <stdint.h>
#include <stuff/base/alts/math.hpp>

#ifdef __STUFF_ALTS_MATH_DIV__
/**
 * @brief 软件整数除法实现。
 * 
 * 把空间从 1.5KB 节约到 48 字节。时间换空间的做法，计算很慢。非必要不开启。
 * 
 * @see <Fast(est) Double-Word Integer Division>
 *      https://skanthak.homepage.t-online.de/integer.html
*/
extern "C" int64_t __divdi3(int64_t dividend, int64_t divisor) {
	bool neg = (dividend < 0) ^ (0 > divisor);
	
	if (dividend < 0) dividend = -dividend;
	if (divisor < 0) divisor = -divisor;

	int64_t quotient = 0;

	while (dividend >= divisor) {
		dividend -= divisor;
		++quotient;
	}
	
	return neg ? -quotient : quotient;
}
#endif

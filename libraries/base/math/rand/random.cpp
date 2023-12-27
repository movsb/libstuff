#include <stuff/base/math/rand/random.hpp>

#include <limits>

namespace ebp {
namespace base {
namespace math {
namespace rand {

static int32_t _seed = 1;

// 一个简单的线性同余生成算法
// https://www.cnblogs.com/shine-lee/p/9516757.html
// Xn+1=(aXn+c) mod m
// 
// 其中，X 为伪随机序列，
// 
// m ，m>0 ，模数，显然其也是生成序列的最大周期
// a ，0<a<m ，乘数
// c ，0≤c<m ，增量
// X0 ，0≤X0<m，种子点（起始值）
// 
static constexpr int32_t _m = std::numeric_limits<int32_t>::max();
static constexpr int32_t _a = 48271;
static constexpr int32_t _c = 0;
static int32_t _linear_congruential_generator_int31() {
	_seed = (int64_t(_a) * _seed + _c) % _m;
	return _seed; 
}

// 后期可以允许修改此伪随机数生成函数的实现
static inline int32_t _int31() {
	return _linear_congruential_generator_int31();
}

void        seed(int32_t seed) { _seed = seed; }
int32_t     int31_()    { return _int31(); }
uint32_t    uint32_()   { return uint32_t(int31_()) >> 15 | uint32_t(int31_()) << 16; }
int16_t     int16_()    { return int16_t(int31_() >> 16); }
uint16_t    uint16_()   { return uint16_t(int31_() >> 15); }

} // namespace rand
} // namespace math
} // namespace base
} // namespace ebp

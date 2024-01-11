#pragma once

#include <stdint.h>
#include <limits>

namespace ebp {
namespace base {
namespace math {
namespace rand {

// 默认已经从 1 初始化，应用无需调用。
void        seed(int32_t seed);
int32_t     int31_();
uint32_t    uint32_();
int16_t     int16_();
uint16_t    uint16_();

} // namespace rand
} // namespace math
} // namespace base
} // namespace ebp

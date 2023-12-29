#include <stdint.h>
#include <inttypes.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace stuff {
namespace base {
namespace time {

void __stuff_sleep_us(int64_t microseconds) {
	uint32_t ticks = microseconds / 1000 / portTICK_PERIOD_MS;
	vTaskDelay(ticks);
}

} // namespace time
} // namespace base
} // namespace stuff

// 如果不定义这个函数（名字随便写），则上面的 __stuff_sleep_us 就无论如何也找不到（链接的时候），
// 也不知道是哪里出问题了，草他妈的。
// 在其它文件里面重复定义也不会报错。
// 搞了三个小时没解决问题。真是日了狗了。
// 编译链接命令都是一样的。
void fuck_fuck_fuck() {}

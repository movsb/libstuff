#pragma once

#include <stdint.h>

namespace stuff {
namespace modules {
namespace infrared {

namespace keycode {

typedef uint8_t KeyCode;

/**
 * @brief 代表无效 KeyCode 的值，应该没有被无端占用吧？
*/
constexpr KeyCode Invalid = 0xFF;

namespace model1 {
	enum {
		Power   = KeyCode(0x45),
		Menu    = KeyCode(0x47),
		Test    = KeyCode(0x44),
		Back    = KeyCode(0x43),
		Up      = KeyCode(0x40),
		Down    = KeyCode(0x19),
		Left    = KeyCode(0x07),
		Right   = KeyCode(0x09),
		OK      = KeyCode(0x15),
		Clear   = KeyCode(0x0D),
		Num0    = KeyCode(0x16),
		Num1    = KeyCode(0x0C),
		Num2    = KeyCode(0x18),
		Num3    = KeyCode(0x5E),
		Num4    = KeyCode(0x08),
		Num5    = KeyCode(0x1C),
		Num6    = KeyCode(0x5A),
		Num7    = KeyCode(0x42),
		Num8    = KeyCode(0x52),
		Num9    = KeyCode(0x4A),
	};
	
	/**
	 * @brief 返回 key 对应的按键数值。
	 * 
	 * @return 返回 [0, 9]。如果不是数值按键，返回 -1.
	*/
	int8_t key2num(KeyCode keyCode);
	
	/**
	 * @brief 把 key 转换成字符串名。
	*/
	const char* key2name(KeyCode keyCode);
} // namespace model1
} // namespace keycode
}
}
}

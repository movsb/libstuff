#include <stuff/base/log.hpp>

using namespace stuff::base;

int main() {
	alts::printf("Literal: %%, %c\n", '%');

	// 布尔值
	alts::printf("%t %t\n", true, false);
	
	// 整数
	{
		int a = 1;
		unsigned b = 2;
		int8_t c = 3;
		uint8_t d = 4;
		int16_t e = 5;
		uint16_t f = 6;
		int32_t g = 7;
		uint32_t h = 8;
		int64_t i = 9;
		uint64_t j = 10;

		alts::printf("%d %d %d %d %d %d %d %d %d %d\n", a, b, c, d, e, f, g, h, i, j);
		alts::printf("%b %b %b %b %b %b %b %b %b %b\n", a, b, c, d, e, f, g, h, i, j);
		alts::printf("%x %x %x %x %x %x %x %x %x %x\n", a, b, c, d, e, f, g, h, i, j);
		alts::printf("%X %X %X %X %X %X %X %X %X %X\n", a, b, c, d, e, f, g, h, i, j);
	}

	alts::printf("hello stuff %s\n");
	
	alts::printf("Unicode: %c %c %c\n", U'桃', U'🍑', U'🍌');
	alts::printf("Unicode: %c\n", 25105);
	alts::printf("Unicode: %c\n", 0xFFFFFFFF);

	return 0;
}

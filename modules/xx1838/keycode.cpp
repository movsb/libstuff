#include "keycode.hpp"

namespace stuff {
namespace modules {
namespace infrared {
namespace keycode {
namespace model1 {

int8_t key2num(KeyCode key) {
	switch (key) {
		case Num0: return 0;
		case Num1: return 1;
		case Num2: return 2;
		case Num3: return 3;
		case Num4: return 4;
		case Num5: return 5;
		case Num6: return 6;
		case Num7: return 7;
		case Num8: return 8;
		case Num9: return 9;
	default:
		return -1;
	}
}

// 为了省点内存，就不定义 map 了，简单 case 下。
const char *key2name(KeyCode keyCode) {
	#define K(N) case N: return #N
	switch (keyCode) {
		K(Power);
		K(Menu);
		K(Test);
		K(Back);
		K(Up);
		K(Down);
		K(Left);
		K(Right);
		K(OK);
		K(Clear);
		K(Num0);
		K(Num1);
		K(Num2);
		K(Num3);
		K(Num4);
		K(Num5);
		K(Num6);
		K(Num7);
		K(Num8);
		K(Num9);
	default:
		return "(unknown)";
	}
	#undef K
}

} // namespace model1
} // namespace keycode
}
}
}

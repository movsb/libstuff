#pragma once

#include <functional>

#include <esp_ota_ops.h>

namespace stuff {
namespace targets {
namespace esp32 {
namespace app {

inline const char* projectName() {
	return ::esp_ota_get_app_description()->project_name;
}

inline const char* appVersion() {
	return ::esp_ota_get_app_description()->version;
}

struct Arch {
	enum Value {
		esp8266,
	};
};

class OTA {
public:
	OTA(Arch::Value arch)
		: _arch(arch)
		{}
	
	void execute(std::function<void()> fn);

protected:
	Arch::Value _arch;
};

}
}
}
}

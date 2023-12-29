#include "timer.hpp"

namespace stuff {
namespace targets {
namespace esp32 {
namespace freertos {
namespace timer {

namespace internal {
	struct Wrapper {
		std::function<void()> callback;
	};
} // namespace internal


static void timerHandler(TimerHandle_t handle) {
	auto w = static_cast<internal::Wrapper*>(pvTimerGetTimerID(handle));
	auto cb = w->callback;
	delete w;
	xTimerDelete(handle, 0);
	cb();
}

void after(const Duration &duration, std::function<void()> callback) {
	auto w = new internal::Wrapper;
	w->callback = callback;
	TimerHandle_t t = xTimerCreate("after", duration.milliseconds() / portTICK_PERIOD_MS, false, w, timerHandler);
	xTimerStart(t, 0);
}

}
}
}
}
}

#include "timer.hpp"

namespace stuff {
namespace targets {
namespace esp32 {
namespace freertos {
namespace timer {

namespace internal {
	struct Wrapper {
		std::function<void()> callback;
		bool isTick;
		TimerHandle_t t;
	};
} // namespace internal


static void timerHandler(TimerHandle_t handle) {
	auto w = static_cast<internal::Wrapper*>(pvTimerGetTimerID(handle));
	auto cb = w->callback;
	if (!w->isTick) {
		delete w;
		xTimerDelete(handle, portMAX_DELAY);
	}
	cb();
}

static internal::Wrapper* _startTimer(const char *name, const Duration &duration, std::function<void()> callback, bool isTick) {
	auto w = new internal::Wrapper;
	w->callback = callback;
	w->isTick = isTick;
	TimerHandle_t t = xTimerCreate(name, duration.milliseconds() / portTICK_PERIOD_MS, isTick, w, timerHandler);
	w->t = t;
	xTimerStart(t, portMAX_DELAY);
	return w;
}

Disposable after(const Duration &duration, std::function<void()> callback) {
	auto w = _startTimer("after", duration, callback, false);
	return [=] {
		xTimerDelete(w->t, portMAX_DELAY);
		delete w;
	};
}

Disposable tick(const Duration &duration, std::function<void()> callback) {
	auto w = _startTimer("tick", duration, callback, true);
	return [=] {
		xTimerDelete(w->t, portMAX_DELAY);
		delete w;
	};
}

}
}
}
}
}

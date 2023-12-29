#pragma once

#include <functional>

#include <freertos/FreeRTOS.h>
#include <freertos/timers.h>

#include <stuff/base/time/duration.hpp>

namespace stuff {
namespace targets {
namespace esp32 {
namespace freertos {
namespace timer {

using stuff::base::time::Duration;

void after(const Duration &duration, std::function<void()> callback);

} // namespace timer
} // namespace freertos
} // namespace esp32
} // namespace targets
} // namespace stuff

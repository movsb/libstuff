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

typedef std::function<void()> Disposable;

/**
 * @brief 在指定的时长后调用回调函数一次。
*/
Disposable after(const Duration &duration, std::function<void()> callback);

/**
 * @brief 周期地调用指定的函数。
*/
Disposable tick(const Duration &duration, std::function<void()> callback);

} // namespace timer
} // namespace freertos
} // namespace esp32
} // namespace targets
} // namespace stuff

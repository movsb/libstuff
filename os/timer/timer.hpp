#pragma once

#include <functional>

#include <stuff/base/time.hpp>

namespace stuff {
namespace os {
namespace timer {

using stuff::base::time::Duration;

typedef std::function<void()> Disposable;

/**
 * @brief 在指定的时长后调用回调函数一次。
 * @note 此函数会立即返回，指定时间达到后，会在新的任务里面调用回调函数。
 *          如果不希望如此，可以考虑 time::after。
*/
Disposable after(const Duration &duration, std::function<void()> callback);

/**
 * @brief 周期地调用指定的函数。
 * @note 此函数会立即返回，指定时间达到后，会在新的任务里面调用回调函数。
 *          如果不希望如此，可以考虑 time::tick。
*/
Disposable tick(const Duration &duration, std::function<void()> callback);

} // namespace timer
} // namespace os
} // namespace stuff

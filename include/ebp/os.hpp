#pragma once

#include <string>
#include <functional>
#include <sys/time.h>

#include "base.hpp"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace ebp {

namespace os {

class TaskManager;

struct __TaskProcWrapper {
	TaskManager *that;
	std::function<void()> func;
};

class TaskManager {
public:
	BaseType_t createTask(std::function<void()> func) {
		return createTask(func, "noname");
	}
	BaseType_t createTask(std::function<void()> func, const char* name) {
		auto w = new __TaskProcWrapper{this, func};
		if (auto err = xTaskCreate(__taskProc, name, 2024, w, 0, nullptr); err != pdPASS) {
			delete w;
			return err;
		}
		return pdPASS;
	}

private:
	static void __taskProc(void *arg) {
		auto w = reinterpret_cast<__TaskProcWrapper*>(arg);
		auto func = w->func;
		delete w;
		return func();
	}
};

static TaskManager __taskManager;

BaseType_t createTask(std::function<void()> func, const char* name);
BaseType_t createTask(std::function<void()> func) {
	return createTask(func, "");
}
BaseType_t createTask(std::function<void()> func, const char* name) {
	return __taskManager.createTask(func, name);
}

void terminateTask() {
	return vTaskDelete(nullptr);
}

} // namespace os

} // namespace ebp

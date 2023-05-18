#pragma once

#include <string>
#include <functional>
#include <sys/time.h>

#include "base.hpp"
#include "io.hpp"

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
		if (auto err = xTaskCreate(__taskProc, name, 4096, w, 0, nullptr); err != pdPASS) {
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
		func();
		::vTaskDelete(nullptr);
	}
};

static TaskManager __taskManager;

BaseType_t createTask(std::function<void()> func, const char* name);
inline BaseType_t createTask(std::function<void()> func) {
	return createTask(func, "");
}
inline BaseType_t createTask(std::function<void()> func, const char* name) {
	return __taskManager.createTask(func, name);
}

inline void terminateTask() {
	return vTaskDelete(nullptr);
}

////////////////////////////////

class _File : public io::ReadWriteCloser {
public:
	_File(FILE* file) : _fp(file), _isStdin(file == stdin) { }
public:
	virtual esp_err_t (close)() override {
		return ::fclose(_fp);
	}
	//
	// TODO: ignore interrupts.
	virtual std::tuple<int, esp_err_t> (read)(void *buf, size_t size) override {
		if (size == 0) {
			return {0, ESP_OK };
		}
		
		// stdin will block forever. always hack it for non-buffered IO.
		// `setvbuf(stdin, NULL, _IONBF, 0);` is ignored.
		if (_isStdin) {
			size = 1;
		}

		size_t n = ::fread(buf, 1, size, _fp);
		if (n == 0) {
			if (::feof(stdin)) {
				return { 0, EOF };
			} else if (::ferror(stdin)) {
				return { 0, ESP_ERR_INVALID_STATE };
			} else {
				return { 0, ESP_FAIL };
			}
		}

		return { int(n), ESP_OK };
	}

	// TODO: ignore interrupts.
	virtual std::tuple<int, esp_err_t> (write)(void *buf, size_t size) override {
		if (size == 0) {
			return { 0, ESP_OK };
		}

		size_t nn = 0;
		
		for (;;) {
			size_t n = ::fwrite(buf, 1, size, _fp);
			if (n > 0) {
				nn += n;
			}
			if (nn == size) {
				return {nn, ESP_OK};
			}
			if (int n = ::ferror(_fp); n != 0) {
				ESP_LOGI("os", "ferror: %d", n);
				return { nn, ESP_FAIL };
			}
			if (n == 0) {
				return { nn, ESP_FAIL };
			}
		}
	}
	
private:
	FILE *_fp;
	bool _isStdin;
};

extern _File &StdIn, &StdOut, &StdErr;

esp_err_t bindStdioWithUART(uint8_t num = 0);

} // namespace os
} // namespace ebp

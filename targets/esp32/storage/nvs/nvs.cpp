#include "nvs.hpp"

#include <nvs_flash.h>
#include <esp_err.h>

namespace stuff {
namespace targets {
namespace esp32 {
namespace storage {
namespace nvs {

#define _nvs_handle reinterpret_cast<nvs_handle>(_impl)

static void _init() {
	ESP_ERROR_CHECK(::nvs_flash_init());
}
static void _uninit() {
	ESP_ERROR_CHECK(::nvs_flash_deinit());
}

NonVolatileStorage::NonVolatileStorage(const char* ns) {
	_init();
	ESP_ERROR_CHECK(::nvs_open(ns, NVS_READWRITE, reinterpret_cast<nvs_handle*>(&_impl)));
}
NonVolatileStorage::~NonVolatileStorage() {
	::nvs_close(_nvs_handle);
	commit();
	_uninit();
}

void NonVolatileStorage::commit() {
	ESP_ERROR_CHECK(::nvs_commit(_nvs_handle));
}

bool NonVolatileStorage::get(const char* key, std::string *value) {
	size_t len;
	if(::nvs_get_str(_nvs_handle, key, nullptr, &len) != ESP_OK) {
		return false;
	}
	auto &v = *value;
	v.resize(len); // len 是包含 0 的。
	if(::nvs_get_str(_nvs_handle, key, &v[0], &len) != ESP_OK) {
		return false;
	}
	v.resize(len - 1); // 移除最后的 0 。
	return true;
}

void NonVolatileStorage::set(const char *key, const char *value) {
	ESP_ERROR_CHECK(::nvs_set_str(_nvs_handle, key, value));
}

} // namespace nvs
} // namespace storage
} // namespace esp32
} // namespace targets
} // namespace stuff

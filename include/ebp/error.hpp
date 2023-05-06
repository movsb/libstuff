#pragma once

#include <string>

#include <esp_err.h>

struct error {
	virtual std::string Error() = 0;
};

// struct ErrorString : error {
// 	std::string s;
	
// 	std::string Error() override { return s; }
// };

// error NewErrorString(const char* str) {
// 	ErrorString e;
// 	e.s = str;
// 	return std::move(e);
// }


constexpr esp_err_t kEOF = 0x10000000, __last = ESP_FAIL;

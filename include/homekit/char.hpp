#pragma once

#include <hap.h>
#include <hap_apple_servs.h>
#include <hap_apple_chars.h>

namespace homekit {

namespace characteristic {

class Base {
public:
	operator hap_char_t*() {
		return _handle;
	 }
	Base() : _handle(nullptr) {}
	virtual ~Base() {
		::hap_char_delete(_handle);
	}
protected:
	hap_char_t *_handle;
};

class On : public Base {
public:
	On(bool on) {
		_handle = ::hap_char_on_create(on);
	}
};

class Name : public Base {
public:
	Name(const char *name) {
		_handle = ::hap_char_string_create(const_cast<char*>(HAP_CHAR_UUID_NAME), HAP_CHAR_PERM_PR, const_cast<char*>(name));
	}
};

} // namespace characteristic
} // namespace homekit

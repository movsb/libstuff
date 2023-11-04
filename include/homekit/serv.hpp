#pragma once

#include <hap.h>
#include <hap_apple_servs.h>
#include <hap_apple_chars.h>

#include "char.hpp"

namespace homekit {

namespace service {

class Base {
public:
	operator hap_serv_t*() {
		return _handle;
	 }
	Base() : _handle(nullptr) {}
	virtual ~Base() {
		::hap_serv_delete(_handle);
	}
	
public:
	int AddCharacteristic(characteristic::Base *c) {
		return ::hap_serv_add_char(*this, *c);
	}
	void SetReadCallback(hap_serv_read_t read) {
		return ::hap_serv_set_read_cb(*this, read);
	}
	void SetWriteCallback(hap_serv_write_t write) {
		return ::hap_serv_set_write_cb(*this, write);
	}
protected:
	hap_serv_t *_handle;
};

class AccessoryInformation : public Base {
public:
	AccessoryInformation(hap_acc_cfg_t *cfg) {
		_handle = ::hap_serv_accessory_information_create(cfg);
	}
};

class LightBulb : public Base {
public:
	LightBulb(bool on) {
		_handle = ::hap_serv_lightbulb_create(on);
	}
};

class Switch : public Base {
public:
	Switch(bool on) {
		_handle= ::hap_serv_switch_create(on);
	}
};

} // namespace service
} // namespace homekit

#pragma once

#include <tuple>
#include <vector>
#include <hap.h>

#include "char.hpp"
#include "serv.hpp"

namespace homekit {
namespace hap {

class Config {
public:
	hap_cfg_t underlying;
	hap_cfg_t* operator->() { return &underlying; }
};

struct UserData {
	void *wrapper;
	void *user;

	UserData(void *wrapper)
		: wrapper(wrapper)
		, user(nullptr)
		{}
};

class Accessory {
public:
	Accessory(hap_acc_cfg_t *cfg) {
		_handle = ::hap_acc_create(cfg);
	}
	~Accessory() {
		::hap_acc_delete(_handle);
	}
	operator hap_acc_t*() { return _handle; }
public:
	int AddService(service::Base *s) {
		return ::hap_acc_add_serv(*this, *s); 
	}
	
public:
	static int Identify(hap_acc_t *a) {
		ESP_LOGI("class Accessory", "accessory identified");
		return HAP_SUCCESS;
	}
protected:
	hap_acc_t *_handle;
};

void SetDebugLevel(hap_debug_level_t level) { return ::hap_set_debug_level(level); }

std::tuple<Config,int> GetConfig() {
	Config c;
	return {c, ::hap_get_config(&c.underlying)};
}

int SetConfig(const Config& c) { return ::hap_set_config(&c.underlying); }

int Init(hap_transport_t transport) { return ::hap_init(transport); }
int Start() { return ::hap_start(); }

void AddAccessory(Accessory *a) { return ::hap_add_accessory(*a); }
void DeleteAllAccessories() { ::hap_delete_all_accessories(); }

void SetSetupCode(const char *setupCode) { return ::hap_set_setup_code(setupCode); }
int SetSetupId(const char *setupId) { return ::hap_set_setup_id(setupId); }

} // namespace hap
} // namespace homekit

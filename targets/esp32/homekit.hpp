#pragma once

#include <cstring>
#include <cstdlib>
#include <functional>

#include <hap.h>
#include <hap_apple_servs.h>
#include <hap_apple_chars.h>

namespace stuff {
namespace targets {
namespace esp32 {
namespace homekit {

void init(const char *setupCode, const char *setupID);

struct CharPtrRef {
	const char *ptr;
	
	CharPtrRef(const char *ptr)
		: ptr(ptr)
		{}

	bool operator==(const CharPtrRef& other) const {
		if(ptr != nullptr && other.ptr != nullptr) {
			return std::strcmp(ptr, other.ptr) == 0;
		}
		return ptr == other.ptr;
	}
};

class Characteristic {
public:
	Characteristic(hap_char_t *hc)
		: _hc(hc)
		{}
public:
	CharPtrRef typeUUID() const {
		return ::hap_char_get_type_uuid(_hc);
	}
public:
	int updateValue(bool value) {
		hap_val_t val = { .b = value, };
		return ::hap_char_update_val(_hc, &val);
	}

protected:
	hap_char_t *_hc;
};

class WriteDatum {
public:
	WriteDatum(hap_write_data_t &ref)
		: _ref(ref) {}
	hap_write_data_t* operator->() const { return &_ref; }
	Characteristic characteristic() { return _ref.hc; }
	
private:
	hap_write_data_t &_ref;
};

class WriteData {
	friend class WriteDatum;
public:
	WriteData(hap_write_data_t write_data[], int count)
		: _data(write_data)
		, _count(count)
		{}
public:
	struct _Iter {
		WriteDatum operator*() const { return that._data[_i]; }
		bool operator!=(const _Iter &other) const { return _i != other._i; }
		_Iter operator++() { ++_i; return *this; }
		WriteData &that;
		int _i;
	};
	_Iter begin() { return {*this, 0}; }
	_Iter end()   { return {*this, _count}; }
private:
	hap_write_data_t *_data;
	int _count;
};

class Service {
public:
	operator hap_serv_t*() const { return _svc; }
protected:
	Service(hap_serv_t *svc)
		: _svc(svc)
		{
			if (_svc == nullptr) { abort(); }
			::hap_serv_set_priv(_svc, this);
			::hap_serv_set_read_cb(_svc, _read);
			::hap_serv_set_write_cb(_svc, _write);
		}
public:
	struct _CharacteristicIterator {
		Characteristic operator*() const { return _ch; }
		bool operator !=(const _CharacteristicIterator &other) { return _ch != other._ch; }
		_CharacteristicIterator operator++() { _ch = ::hap_char_get_next(_ch); return *this; }
		hap_char_t *_ch;
	};
	struct _CharacteristicsIterable {
		_CharacteristicIterator begin() { return { ::hap_serv_get_first_char(service) }; }
		_CharacteristicIterator end() { return { nullptr }; }
		Service &service;
	};
	_CharacteristicsIterable characteristics() {
		return { *this };
	}
protected:
	virtual void onWrite(WriteData writes) { }
	virtual hap_status_t onRead(Characteristic hc) { return HAP_STATUS_RES_ABSENT; }
private:
	static int _read (hap_char_t *hc, hap_status_t *status_code, void *serv_priv, void *read_priv) {
		auto svc = reinterpret_cast<Service*>(serv_priv);
		 *status_code = svc->onRead(Characteristic{hc});
		return *status_code == HAP_STATUS_SUCCESS ? HAP_SUCCESS : HAP_FAIL;
	}
	static int _write(hap_write_data_t write_data[], int count, void *serv_priv, void *write_priv) {
		auto svc = reinterpret_cast<Service*>(serv_priv);
		for (int i = 0; i < count; i++) {
			*write_data[i].status = HAP_STATUS_RES_ABSENT;
		}
		bool allTrue = true;
		svc->onWrite(WriteData(write_data, count));
		for (int i = 0; i < count; i++) {
			if (*write_data[i].status != HAP_STATUS_SUCCESS) {
				allTrue = false;
				break;
			}
		}
		return allTrue ? HAP_SUCCESS : HAP_FAIL;
	}
protected:
	hap_serv_t *_svc;
};

class Accessory {
public:
	Accessory(
		hap_cid_t cid,
		const char *name,
		const char *model,
		const char *manufacturer,
		const char *serialNumber,
		const char *firmwareRevision
	)
	{
		hap_acc_cfg_t cfg = {
			.name               = const_cast<char*>(name),
			.model              = const_cast<char*>(model),
			.manufacturer       = const_cast<char*>(manufacturer),
			.serial_num         = const_cast<char*>(serialNumber),
			.fw_rev             = const_cast<char*>(firmwareRevision),
			.hw_rev             = nullptr,
			.pv                 = const_cast<char*>("1.1.0"),
			.cid                = cid,
			.identify_routine   = [](hap_acc_t *) { return HAP_SUCCESS; }
		};
		_acc = ::hap_acc_create(&cfg);
		if (_acc == nullptr) { abort(); }
	}
	operator hap_acc_t*() const { return _acc; }
public:
	void addService(Service *service) {
		if (::hap_acc_add_serv(_acc, *service) != HAP_SUCCESS) {
			abort();
		}
	}
protected:
	hap_acc_t *_acc;
};

inline void addAccessory(Accessory *accessory) {
	::hap_add_accessory(*accessory);
}

inline void start() {
	if (::hap_start() != HAP_SUCCESS) {
		abort();
	}
}

class Lightbulb : public Service {
public:
	Lightbulb(bool on)
		: Service(::hap_serv_lightbulb_create(on))
	{}
protected:
	virtual hap_status_t onRead(homekit::Characteristic ch) override {
		if (ch.typeUUID() == HAP_CHAR_UUID_ON) {
			if (_onReadOn) {
				ch.updateValue(_onReadOn());
				return HAP_STATUS_SUCCESS;
			}
		}
		return HAP_STATUS_RES_ABSENT;
	}
	virtual void onWrite(homekit::WriteData writes) override {
		for (auto write : writes) {
			auto ch = write.characteristic();
			if (ch.typeUUID() == HAP_CHAR_UUID_ON) {
				if(_onWriteOn) {
					_onWriteOn(write->val.b);
					// 总是假定 Write 会成功。
					ch.updateValue(write->val.b);
					*write->status = HAP_STATUS_SUCCESS;
				}
			}
		}
	}
public:
	void onReadOn(std::function<bool()> fn) {
		_onReadOn = fn;
	}
	void onWriteOn(std::function<void(bool on)> fn) {
		_onWriteOn = fn;
	}
	void writeOn(bool on) {
		for (auto ch : characteristics()) {
			if (ch.typeUUID() == HAP_CHAR_UUID_ON) {
				ch.updateValue(on);
			}
		}
	}
protected:
	std::function<bool()>           _onReadOn;
	std::function<void(bool on)>    _onWriteOn;
};

inline void resetPairings() {
	::hap_reset_pairings();
}

}
}
}
}

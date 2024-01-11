#include "nvs.hpp"

#include <nvs_flash.h>
#include <esp_err.h>
#include <esp_log.h>

#include <string>
#include <vector>

namespace stuff {
namespace targets {
namespace esp32 {
namespace storage {
namespace nvs {

#define _nvs_handle                 reinterpret_cast<nvs_handle>(_impl)

static void _init() {
	ESP_ERROR_CHECK(::nvs_flash_init());
}
// static void _uninit() {
// 	ESP_ERROR_CHECK(::nvs_flash_deinit());
// }

NonVolatileStorage::NonVolatileStorage() {
	_init();
	ESP_ERROR_CHECK(::nvs_open(NVS_DEF_NS, NVS_READWRITE, reinterpret_cast<nvs_handle*>(&_impl)));
}
NonVolatileStorage::~NonVolatileStorage() {
	commit();
	// _uninit();
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

bool NonVolatileStorage::get(const char* key, bool useDef, std::string*value) {
	if (get(key, value)) {
		return true;
	}
	if (useDef) {
		auto d = findDefault(key, Value::SZ);
		*value = d.s;
		return true;
	}
	return false;
}

bool NonVolatileStorage::get(const char* key, bool useDef, uint8_t *value) {
	auto err = ::nvs_get_u8(_nvs_handle, key, value);
	if (err == ESP_OK) { return true; }
	else if (err == ESP_ERR_NVS_NOT_FOUND) {
		if (useDef) {
			auto d = findDefault(key, Value::U8);
			*value = static_cast<uint8_t>(d.u);
			return true;
		}
	}
	return false;
}

void NonVolatileStorage::set(const char *key, const char *value) {
	ESP_ERROR_CHECK(::nvs_set_str(_nvs_handle, key, value));
	commit();
}

/**
 * @brief 通配符匹配。
 * 
 * 只支持 * 号。
*/
bool match(const char *pattern, const char *input) {
	if (!*pattern) {
		if (!*input) {
			return true;
		}
		return false;
	}
	if (*pattern != '*') {
		if (*pattern == *input) {
			return match(pattern+1, input+1);
		}
		return false;
	}
	for (int i = 0; i <= std::strlen(input); i++) {
		if (match(pattern+1, input+i)) {
			return true;
		}
	}
	return false;
}

void NonVolatileStorage::each(const char* part, const char* ns, const char* keyLike, std::function<void(const char* part, const char* ns, const char *key, const Value &val)> callback) {
	Value val;
	std::string tmp;
	
	// 遍历和读、写都会加锁，不得不先把所有的 keys 存起来了，有点费空间？
	struct Cache {
		Cache(const char* part, const char *ns, const char*key,  __nvs_ItemType ty)
			: part(part), ns(ns), key(key), ty(ty) {}
		const char* part;
		const char* ns;
		std::string key;
		__nvs_ItemType ty;
	};
	
	std::vector<Cache> _tmp;
	
	// part:ns
	struct PNN {
		const char* part;
		const char* ns;
		PNN(const char* part, const char* ns)
			: part(part), ns(ns) {}
		bool operator==(const PNN& that) const {
			return part == that.part && ns == that.ns;
		}
		struct Hasher {
			std::size_t operator()(const PNN& that) const {
				return static_cast<std::size_t>(
					reinterpret_cast<std::size_t>(that.part)
					^ reinterpret_cast<std::size_t>(that.ns)
				);
			}
		};
	};
	
	struct Handle{
		Handle() : h(nvs_handle{}) {}
		Handle(Handle&& other) : h(other.h) { other.h = nvs_handle{}; }
		Handle(nvs_handle h) : h(h) {}
		~Handle() {
			if (h) {
				nvs_close(h);
				h = nvs_handle{};
			}
		}
		nvs_handle h;
	};

	std::unordered_map<PNN, Handle, PNN::Hasher> _handles;
	
	auto err = ::__nvs_for_each(part, ns, [&](const char* part, const char* ns, const char* key, __nvs_ItemType ty) {
		if (match(keyLike, key)) {
			_tmp.emplace_back(part, ns, key, ty);
		}
	});
	ESP_ERROR_CHECK(err);

	for(auto &item : _tmp) {
		auto part = item.part;
		auto ns = item.ns;
		auto key = item.key.c_str();
		auto ty = item.ty;
		
		if (auto it = _handles.find({part, ns}); it == _handles.end()) {
			nvs_handle h;
			if(::nvs_open_from_partition(part, ns, NVS_READONLY, &h) != ESP_OK) {
				continue;
			}
			_handles.try_emplace({part, ns}, h);
		}
		
		#define _nvs_handle_of(part, ns) _handles[{part, ns}].h
		#define __CASE_I(t1, t2, t3) \
			case __nvs_ItemType::t1: { \
				t2 i; \
				ESP_ERROR_CHECK(:: t3 (_nvs_handle_of(part, ns), key, &i)); \
				val.ty = Value::t1; \
				val.i = i; \
				callback(part, ns, key, val); \
			} \
			break
		#define __CASE_U(t1, t2, t3) \
			case __nvs_ItemType::t1: { \
				t2 u; \
				ESP_ERROR_CHECK(:: t3 (_nvs_handle_of(part, ns), key, &u)); \
				val.ty = Value::t1; \
				val.u = u; \
				callback(part, ns, key, val); \
			} \
			break
		#define __CASE_S_AND_B(ty1, fn, ty2) \
			case __nvs_ItemType::ty1: { \
				val.ty = Value::ty2; \
				size_t size; \
				ESP_ERROR_CHECK(::fn(_nvs_handle_of(part, ns), key, nullptr, &size)); \
				tmp.resize(size); \
				ESP_ERROR_CHECK(::fn(_nvs_handle_of(part, ns), key, &tmp[0], &size)); \
				if (ty == __nvs_ItemType::SZ) { \
					tmp.resize(size - 1); \
					val.s = tmp.c_str(); \
				} else { \
					val.blob.d = tmp.c_str(); \
					val.blob.size = size; \
				} \
				callback(part, ns, key, val); \
			} \
			break
		switch (ty) {
			__CASE_I(I8,  int8_t,   nvs_get_i8);
			__CASE_I(I16, int16_t,  nvs_get_i16);
			__CASE_I(I32, int32_t,  nvs_get_i32);
			__CASE_I(I64, int64_t,  nvs_get_i64);
			__CASE_U(U8,  uint8_t,  nvs_get_u8);
			__CASE_U(U16, uint16_t, nvs_get_u16);
			__CASE_U(U32, uint32_t, nvs_get_u32);
			__CASE_U(U64, uint64_t, nvs_get_u64);
			__CASE_S_AND_B(SZ,   nvs_get_str,  SZ);
			__CASE_S_AND_B(BLOB, nvs_get_blob, BLOB);
			case __nvs_ItemType::ANY:
				abort();
		}
		#undef __CASE_I
		#undef __CASE_U
		#undef __CASE_S_AND_B
	}
}

} // namespace nvs
} // namespace storage
} // namespace esp32
} // namespace targets
} // namespace stuff

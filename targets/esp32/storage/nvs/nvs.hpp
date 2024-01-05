#pragma once

#include <cstring>
#include <string>
#include <functional>
#include <unordered_map>

namespace stuff {
namespace targets {
namespace esp32 {
namespace storage {
namespace nvs {

struct Value {
	// Copy from nts/nvs_flash/src/nvs_types.hpp
	enum Type : uint8_t {
		U8   = 0x01,
		I8   = 0x11,
		U16  = 0x02,
		I16  = 0x12,
		U32  = 0x04,
		I32  = 0x14,
		U64  = 0x08,
		I64  = 0x18,
		SZ   = 0x21,
		BLOB = 0x41,
		ANY  = 0xff
	};
	Value()                             : ty(ANY)           {}
	Value(int64_t i, Type ty)           : i(i), ty(ty)      {}
	Value(uint64_t u, Type ty)          : u(u), ty(ty)      {}
	Value(const char *s)                : s(s), ty(SZ)      {}
	Value(const void *d, size_t size)                       { blob.d = d, blob.size = size; }
	
	const char* typeString() const;
	std::string toString()   const;
	
	union {
		int64_t i;
		uint64_t u;
		const char *s;
		struct {
			const void *d;
			size_t size;
		} blob;
	};
	Type ty;
};

class NonVolatileStorage {
public:
	NonVolatileStorage();
	virtual ~NonVolatileStorage();
public:
	bool get(const char* key, std::string *value);
	void set(const char* key, const char *value);
	/**
	 * @note 默认会 commit。
	*/
	void set(const char* key, const std::string &value) {
		return set(key, value.c_str());
	}
	void commit();
public:
	void each(const char* part, const char* ns, const char* keyLike, std::function<void(const char* part, const char* ns, const char *key, const Value &val)> callback);
private:
	void *_impl;
};

} // namespace nvs
} // namespace storage
} // namespace esp32
} // namespace targets
} // namespace stuff

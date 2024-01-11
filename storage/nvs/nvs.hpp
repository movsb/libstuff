#pragma once

#include <cstring>
#include <string>
#include <functional>
#include <vector>
#include <unordered_map>

#include "value.hpp"
#include "user.hpp"

#define NVS_DEF_PART "nvs"
#define NVS_DEF_NS   "nvs"

namespace stuff {
namespace storage {
namespace nvs {

class NonVolatileStorage {
public:
	NonVolatileStorage();
	virtual ~NonVolatileStorage();
public:
	bool get(const char* key, std::string *value);
	bool get(const char* key, bool useDefault, std::string *value);
	bool get(const char* key, bool useDefault, uint8_t *value);
public:
	void addDefaults(const user::Default *defaults) {
		_defaults.push_back(defaults);
	}
	Value findDefault(const char* key, Value::Type ty) {
		Value value;
		eachDefault([&](const user::Default &def) {
			if (std::strcmp(def.key, key) == 0 && (def.value.ty == ty || ty == Value::ANY)) {
				value = def.value;
				return false;
			}
			return true;
		});
		return value;
	}
	void eachDefault(std::function<bool(const user::Default &def)> callback) {
		for (auto d : _defaults) {
			while (d && d->key) {
				if (!callback(*d)) {
					return;
				}
				d++;
			}
		}
	}
	
public:
	void set(const char* key, const char *value);
	template<typename T>
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
	std::vector<const user::Default*> _defaults;
};

bool match(const char *pattern, const char *input);

} // namespace nvs
} // namespace storage
} // namespace stuff

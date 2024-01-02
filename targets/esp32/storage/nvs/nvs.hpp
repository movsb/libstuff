#pragma once

#include <string>

namespace stuff {
namespace targets {
namespace esp32 {
namespace storage {
namespace nvs {

class NonVolatileStorage {
public:
	NonVolatileStorage(const char* ns);
	virtual ~NonVolatileStorage();
public:
	bool get(const char* key, std::string *value);
	void set(const char* key, const char *value);
	void set(const char* key, const std::string &value) {
		return set(key, value.c_str());
	}
	void commit();
private:
	void *_impl;
};

} // namespace nvs
} // namespace storage
} // namespace esp32
} // namespace targets
} // namespace stuff

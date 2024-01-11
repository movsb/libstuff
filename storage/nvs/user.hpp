#pragma once

#include "value.hpp"

namespace stuff {
namespace storage {
namespace nvs {
namespace user {

/**
 * @brief 用户默认值。
*/
struct Default {
	const char  *key;
	const char  *description;
	nvs::Value  value;

	Default(const char *key, const nvs::Value &value, const char *description)
		: key(key), description(description), value(value) { }
};

inline const Default last() {
	return {nullptr, Value{}, nullptr};
}

} // namespace user
}
}
}

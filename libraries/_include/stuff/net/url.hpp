#pragma once

#include <unordered_map>
#include <string>

namespace stuff {
namespace net {
namespace url {

/**
 * @brief 键值映射。通常用于查询参数和表单数据。
 * 
 * @note 不同于 HTTP 头部，这里的键是大小写相关的。
 * @todo 简单使用场景，暂时未支持单键-多值映射。
*/
class Values {
public:
	Values() {}
	Values(Values&& other) {
		_values = std::move(other._values);
	}
public:
	std::string get(const char *key) {
		auto it = _values.find(key);
		if (it != _values.end()) {
			return it->second;
		}
		return "";
	}
	void set(const char *key, const char *value) {
		if (key && *key && value) {
			_values[key] = value;
		}
	}
	void set(std::string &&key, std::string &&value) {
		if (!key.empty()) {
			_values[key] = value;
		}
	}
	bool has(const char *key) {
		const auto it = _values.find(key);
		return it != _values.cend();
	}
public:
	auto begin() const { return _values.begin(); }
	auto end() const { return _values.end(); }
protected:
	std::unordered_map<std::string, std::string> _values;
};

std::tuple<Values, bool> parseQuery(const char *query);

} // namespace url
} // namespace net
} // namespace stuff

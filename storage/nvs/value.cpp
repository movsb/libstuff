#include "value.hpp"

#include <string>
#include <vector>

namespace stuff {
namespace storage {
namespace nvs {

const char* Value::typeString() const {
	switch (ty) {
		case I8:    return "int8";
		case I16:   return "int16";
		case I32:   return "int32";
		case I64:   return "int64";
		case U8:    return "uint8";
		case U16:   return "uint16";
		case U32:   return "uint32";
		case U64:   return "uint64";
		case SZ:    return "string";
		case BLOB:  return "blob";
		default:    return "unknown";
	}
}

std::string Value::toString() const {
	std::string t;
	char buf[64];
	switch (ty) {
	case Value::I8:
	case Value::I16:
	case Value::I32:
	case Value::I64:
		std::snprintf(buf, sizeof(buf), "%lld", i);
		t = buf;
		break;
	case Value::U8:
	case Value::U16:
	case Value::U32:
	case Value::U64:
		std::snprintf(buf, sizeof(buf), "%llu", u);
		t = buf;
		break;
	case Value::SZ:
		t = s;
		break;
	case Value::BLOB:
		{
			auto toHex = [](uint8_t n) {
				return n <= 9
					? n -  0 + '0'
					: n - 10 + 'A';
			};

			t.resize(blob.size*2);
			char *p = &t[0];
			for (size_t i = 0; i < blob.size; i++) {
				auto b = static_cast<const uint8_t*>(blob.d)[i];
				p[0] = toHex(b >> 4);
				p[1] = toHex(b & 15);
				p += 2;
			}
			break;
		}
	default:
		t = "(?:unknown_value_type)";
	}
	return std::move(t);
};
	
} // namespace nvs
} // namespace storage
} // namespace stuff

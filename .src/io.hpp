#pragma once

#include <tuple>

namespace stuff {
namespace base {
namespace io {

struct Reader {
	virtual std::tuple<int, esp_err_t> (read)(void *buf, size_t size) = 0;
};

struct Writer {
	virtual std::tuple<int, esp_err_t> (write)(void *buf, size_t size) = 0;
};

struct Closer {
	virtual esp_err_t close() = 0;
};

struct ReadWriter       : Reader, Writer { };
struct ReadCloser       : Reader, Closer { };
struct WriteCloser      : Writer, Closer { };
struct ReadWriteCloser  : Reader, Writer, Closer { };

std::tuple<int, esp_err_t> copy(Writer *dst, Reader& src);

class StringReader : public Reader {
public:
	StringReader(const std::string &s)
		: _s(s)
		, _i(0)
		{}
public:
	virtual std::tuple<int, esp_err_t> (read)(void *buf, size_t size) override {
		if (_i >= _s.size()) {
			return { 0, EOF };
		}
		
		int min = std::min(size, _s.size() - _i);
		std::memcpy(buf, &_s[_i], min);
		_i += min;
		return { min, ESP_OK };
	}
	
private:
	std::string _s;
	int _i;
};

class _EofReader : public Reader {
public:
	virtual std::tuple<int, esp_err_t> (read)(void *buf, size_t size) override {
		return { 0, EOF };
	}
};

extern _EofReader _eofReader;
extern Reader* EofReader;

}
}
}

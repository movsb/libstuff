#include <cmath>

#include "decoder.hpp"

namespace stuff {
namespace modules {
namespace infrared {
namespace decoder {
namespace nec {

static inline bool almost(uint16_t  desired, uint16_t current, uint16_t error) {
	return std::abs(desired - current) < error;
}

// 低、高电平允许的误差范围内算有效。
#define lo(d) (almost(_previousDuration, (d), 300))
#define hi(d) (almost(duration, (d), 300))

void Decoder::decode(bool level, uint16_t duration) {
	if (_state == State::invalid) {
		return;
	}
	if (_state == State::end2) {
		_state = State::invalid;
		return;
	}
	
	// 收到低电平，可能是结束码，可能是零一值。
	// 如果是后者，下一次收到高电平时再作是哪种码的判断。
	if (!level) {
		if (_state == State::end1) {
			_state = State::end2;
			return;
		} else {
			_previousDuration = duration;
			return;
		}
	}
	
	// 处理高电平
	switch (_state) {
	default:
		_state = State::invalid;
		return;
	case State::none:
		// 引导码
		if (lo(9000) && hi(4500)) {
			_state = State::start;
			return;
		}
		// 重复码
		// if (lo(9000) && hi(2250)) {
		// 	_state = State::end;
		// 	return;
		// }
		_state = State::invalid;
		return;
	case State::start:
		_state = State::data;
		return decode(level, duration);
	case State::data:
		// 判断是 零值 还是 一值
		if (lo(560) && hi(560)) {
			_buf[_n] >>= 1;
			_buf[_n] &= 0x7F;
		} else if (lo(560) && hi(1680)) {
			_buf[_n] >>= 1;
			_buf[_n] |= 0x80;
		}
		
		if(++_b == 8) {
			_b = 0;
			if(++_n == 4) {
				_state = State::end1;
			}
		}

		return;
	}
}

Decoded Decoder::data() {
	if (_state == State::end2) {
		bool valid = _buf[0]+_buf[1] == 255 && _buf[2]+_buf[3] == 255;
		if (valid) {
			// 复位内部状态。
			// 但是，可能后续有重复键产生，内部数据不复位。
			_reset(false);
			return { _buf[0], _buf[2],  true };
		}
	}

	return {0xFF, Invalid, false};
}

} // namespace nec
} // namespace decoder
} // namespace infrared
} // namespace modules
} // namespace stuff

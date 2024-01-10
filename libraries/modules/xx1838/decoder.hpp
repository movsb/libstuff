#pragma once

#include <stdint.h>
#include <string>
#include <cstring>
#include <cmath>

#include "keycode.hpp"

namespace stuff {
namespace modules {
namespace infrared {
namespace decoder {

/**
 * @brief 红外解码器接口。
 * 
 * 该接口同时支持有操作系统和裸机。
 * 区别在于：
 *   * 裸机可以在电平变化的时候死等电平结束，立马就能知道该电平的持续时间。
 *     所以如果仅仅是写裸机，代码可能会简单很多。
 *   * 操作系统则不一样，只能在电平变化的时候记录发生的时刻，不能死等。
 *     轻则浪费 CPU，重则可能被看门狗复位。
 * 
 * @see file:///红外遥控开关/main.c#L124-L161
 * @see <https://github.com/movsb/circuits/blob/6174da47714c00d626c9837e1dcfaaa30dc84757/%E7%BA%A2%E5%A4%96%E9%81%A5%E6%8E%A7%E5%BC%80%E5%85%B3/main.c#L124-L161>
 *      (不一定有权限)
 *
 * @note 每种解码器返回的数据不一样，没有定义具体的返回值接口。
*/
class Decoder {
public:
	Decoder() {}
	virtual ~Decoder() {}
public:
	/**
	 * @brief 为了复用对象，允许复位对象内部状态。
	*/
	virtual void reset() = 0;
	
	/**
	 * @brief 解码下一个电平时长。
	 * 
	 * @param level     电平的高、低。
	 * @param duration  时长（时值）。以微秒为单位。
	*/
	virtual void decode(bool level, uint16_t duration) = 0;
};

namespace nec {

using namespace keycode;

class Decoded {
public:
	uint8_t userCode;   // 用户码
	KeyCode keyCode;    // 按键码
	// 重复码误产生的概率太高了，忽略
	// bool    repeating;  // 是否重复按键？
public:
	Decoded(uint8_t userCode, KeyCode keyCode, bool valid)
		: userCode(userCode), keyCode(keyCode), _valid(valid)
		{}
	operator bool() const { return _valid; }
private:
	bool _valid;
};

/**
 * @brief NEC 格式解码器。
 *
 * NEC 载波频率为 38Khz
 * 
 *  引导码:    9ms    低电平  + 4.5ms     高电平
 *  零码、:    0.56ms 低电平  + 0.56ms    高电平
 *  一码、:    0.56ms 低电平  + 1.68ms    高电平
 *  结束码:    0.56ms 低电平
 * 
 *  数据帧格式：引导码 + 地址 + 地址反码 + 键值 + 键值反码 + 结束码
 *  重复帧格式：9ms 低电平 + 2.25ms高电平 + 结束位
 *
 *  低位（lsb）在前，即首先收到的是低位的数据
 * 
 * @note 有些模块的高低电平是相反的。
 * @note 重复键误产生的概率太大了，内部禁止了。
*/
class Decoder : public decoder::Decoder {
public:
	Decoder() {
		reset();
	}
public:
	virtual void reset() override {
		_reset(true);
	}

	virtual void decode(bool level, uint16_t duration) override;

	/**
	 * @brief 获取当前的用户码、键码。
	 * 
	 * @note 一旦返回 true，内部会清空按键状态，即：非幂等调用。
	 *          为的是方便马上进入下一个状态解码，而用户不需手动再调用 reset()。
	*/
	Decoded data();
protected:
	void _reset(bool clear) {
		_state = State::none;
		_n = 0;
		_b = 0;
		_previousDuration = 0;
		if (clear) {
			std::memset(_buf, 0, sizeof(_buf));
		}
	}
protected:
	/**
	 * @brief 当前解码状态。
	 * 
	 * @note    \p end1 是数据接收完成后。
	 *          \p end2 是结束码收到后。
	*/
	enum class State { invalid, none, start, data, end1, end2, };

protected:
	State _state;   // 解码状态
	uint8_t _n:4;   // 当前解码第几个字节
	uint8_t _b:4;   // 当前解码字节的第几位（高位在前）

	// 前一次的（低）电平的时值。每两次电平（一低一高）作一次判断。
	// 内部并没有保存上一次是什么电平状态，所以外部应该确保 decode() 时
	// 传入的参数总是一低一高的。不过，就算不是，只能表示数据错了，没啥影响。
	uint16_t _previousDuration;

protected:
	uint8_t _buf[4];
};

} // namespace nec
} // namespace decoder
} // namespace infrared
} // namespace modules
} // namespace stuff

#pragma once

#include <stdint.h>
#include <string>
#include <cmath>
#include <bitset>
#include <map>

// 红外信号都是高、低持续若干时长的电平信号。
class InfraredDecoder {
public:
	InfraredDecoder(bool highFirst) : _highFirst(highFirst)
	{}
	
public:
	// 解码下一个电平信号。
	// level: 高电平还是低电平
	// duration: 电平持续时间
	virtual bool decode(bool level, int32_t duration) = 0;

	// 为了复用对象，允许复位对象内部状态.
	virtual void reset() = 0;
	
	// 当前数据是否完整、有效？
	virtual bool valid() = 0;
	
	// 输出当前按键
	virtual std::string key() const = 0;
	
	bool highFirst() const {
		return _highFirst;
	}
protected:
	bool almost(int32_t duration, int32_t expected) {
		return std::abs(duration - expected) < 300;
	}

	bool isLead(bool level) const {
		return (_highFirst && level) || (!level);
	}

protected:
	bool _highFirst;
};

class GreeDecoder : public InfraredDecoder {
public:
	GreeDecoder(bool highFirst) : InfraredDecoder(highFirst) {}
public:
	virtual bool decode(bool level, int32_t duration) override;
	virtual void reset() override;
	virtual bool valid() override;
	virtual std::string key() const override;
	
protected:
	enum class State {
		invalid, none, start, bits1, conn, bits2, done,
	};

	char const* toString(State state) const {
		static std::map<State, char const*> names = {
			{State::invalid, "invalid"},
			{State::none, "none"},
			{State::start, "start"},
			{State::bits1, "bits1"},
			{State::conn, "conn"},
			{State::bits2, "bits2"},
			{State::done, "done"},
		};
		if (auto it = names.find(state); it != names.end()) {
			return it->second;
		}
		return "unknown";
	}
	
private:
	void changeState(State state);

	// 当前解码状态
	State       _state;
	// 当前已经读取多少位数据了？高位在前。
	int8_t      _bit;
	// 高电平持续了多久？
	int32_t     _previousDuration;

private:
	std::bitset<35> _bits1;
	std::bitset<32> _bits2;
};

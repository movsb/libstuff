#include <stdio.h>
#include <cmath>
#include <unordered_map>
#include <sys/time.h>

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "decoder.h"

/*
NEC 载波频率为 38Khz

	引导码:    9ms    高电平  + 4.5ms     低电平
	一码、:    0.56ms 高电平  + 0.56ms    低电平
	零码、:    0.56ms 高电平  + 1.68ms    低电平
	结束码:    0.56ms 高电平

数据帧格式：引导码 + 地址 + 地址反码 + 键值 + 键值反码 + 结束码
重复帧格式：9ms 高电平 + 2.25ms低电平 + 结束位 + 结束码
高位在前，即首先收到的是高位的数据

注：有些模块的高低电平是相反的。
*/
class NecInfraredDecoder : public InfraredDecoder {
public:
	NecInfraredDecoder(bool highFirst): InfraredDecoder(highFirst) { }

public:
	virtual bool decode(bool level, int32_t duration) override {
		if (_state == State::invalid) {
			puts("无效状态1");
			return false;
		} else if (_state == State::done) {
			return false;
		}
		if (isLead(level)) {
			_previousDuration = duration;
			return true;
		}

		switch (_state) {
		default:
			// 无效状态。
			_state = State::invalid;
			// printf("无效状态2\n");
			return false;
		case State::none:
			if (almost(_previousDuration, 9000) && almost(duration, 4500)) {
				// 引导码
				_state = State::start;
				// puts("解码到引导码");
				return true;
			} else if (almost(_previousDuration, 9000) && almost(duration, 2250)) {
				// 重复按键信号
				_state = State::done;
				_repeatLast = true;
				return false;
			}
			_state = State::invalid;
			// 一般是遥控器没对准接收器（中途产生的信号）。
			// printf("无效状态3：%d, %d\n", int(_previousDuration), int(duration));
			return false;
		case State::start:
			// 地址码开始了……
			_state = State::data;
			return decode(level, duration);
		case State::data:
			if (almost(_previousDuration, 560) && almost(duration, 560)) {
				// 处理 1
				_data[_bytes -1 - _bit / 8] |= 1 << _bit % 8;
				// fallthrough
			} else if (almost(_previousDuration, 560) && almost(duration, 1680)) {
				// 处理 0
				// 啥也不用干。
				// fallthrough
			} else {

			}
			
			// printf("解码到第 %d 位数据：%d\n", _bit, int(duration));
			
			--_bit;

			if (_bit < 0) {
				_state = State::done;
				return false;
			}
			
			return true;
		case State::done:
			printf("有多余的数据\n");
			return false;
		}
	}
	
	virtual void reset() override {
		_state = State::none;
		_bit = _bytes * 8 -1;
		for (int i = 0; i < _bytes; ++i) {
			_data[i] = 0;
		}
		_repeatLast = false;
	}

	virtual bool valid() override {
		if (_state != State::done) {
			return false;
		}
		// 先忽略重复键，产生次数太高了。
		if (_repeatLast) {
			return false;
		}
		if (_data[0] + _data[1] == 255 && _data[2] + _data[3] == 255) {
			return true;
		}
		return false;
	}
	
	uint8_t address() const {
		return _data[0];
	}

	uint8_t keyCode() const {
		return _data[3];
	}
	
	bool repeat() const {
		return _repeatLast;
	}
	
	char const* keyName() const {
		if (auto it = _keyNames.find(keyCode()); it != _keyNames.end()) {
			return it->second;
		}
		return "(unknown key code)";
	}
	
	virtual std::string key() const override {
		char buf[128];
		snprintf(&buf[0], sizeof(buf)/sizeof(buf[0]),
			"Address: 0x%02X, KeyCode: 0x%02X, KeyName: %s",
			address(), keyCode(), keyName()
		);
		return buf;
	}
	
private:
protected:
	enum class State {
		invalid, none, start, data, done,
	};
	
	// 当前解码状态
	State       _state;
	// 当前数据位在哪一位。 31~0
	int8_t      _bit;
	// 高电平持续了多久？
	int32_t     _previousDuration;
	
	// 是否是重复上次按键？
	bool        _repeatLast;

private:
	static constexpr int8_t _bytes = 4;
	uint8_t                 _data[_bytes];
	
private:
	static const std::unordered_map<uint8_t, char const*> _keyNames;
};

const std::unordered_map<uint8_t, char const*> NecInfraredDecoder::_keyNames = {
	{0xA2, "CH-" },
	{0x62, "CH"  },
	{0xE2, "CH+" },
	{0x22, "⏮"  },
	{0x02, "⏭"  },
	{0xC2, "⏯"  },
	{0xE0, "-"   },
	{0xA8, "+"   },
	{0x90, "EQ"  },
	{0x68, "0"   },
	{0x98, "100+"},
	{0xB0, "200+"},
	{0x30, "1"   },
	{0x18, "2"   },
	{0x7A, "3"   },
	{0x10, "4"   },
	{0x38, "5"   },
	{0x5A, "6"   },
	{0x42, "7"   },
	{0x4A, "8"   },
	{0x52, "9"   }
};


static InfraredDecoder *sgDecoders[] = {
	new NecInfraredDecoder(false),
	new GreeDecoder(false),
};


int64_t get_absolute_time() {
	struct timeval tv_now;
	gettimeofday(&tv_now, NULL);
	int64_t time_us = (int64_t)tv_now.tv_sec * 1000000L + (int64_t)tv_now.tv_usec;
	return time_us;
}


typedef struct {
	int64_t now;
	bool level;
} Wave;


void gpio_callback(Wave *w) {
	// 静态变量，上一次的时间。
	static int64_t last = 0;

	auto diff = w->now - last;
	last = w->now;
	
	bool high = w->level;
	
	// printf("diff is %lld, high is %d\n", diff, int(high));

	// 进入时判断是否重新开始（低电平最多不会持续 4.5ms）。
	// 设置长一点儿可以避免很多无效数据。
	if (diff > 20000) {
		for(auto& d : sgDecoders) {
			d->reset();
		}
		return;
	}

	// 解码前一个电平。
	// 即：进入高电平时，解码前一个低电平；
	//    进入低电平时，解码前一个高电平。
	for (auto & d : sgDecoders) {
		d->decode(!high, int32_t(diff));
	}
	
	// 高低码，结束码后会被 IO 口下拉，以此表示结束。
	// 低高码，结束码后会被 IO 口上拉，以此表示结束。
	if ((sgDecoders[0]->highFirst() && !high) || (!sgDecoders[0]->highFirst() && high)) {
		for (auto &d : sgDecoders) {
			if (d->valid()) {
				puts(d->key().c_str());
			}
		}
	}
}

#define PIN GPIO_NUM_19

static QueueHandle_t gpio_evt_queue = NULL;

void IRAM_ATTR gpio_isr_handler(void *data) {
	Wave w = {
		.now = get_absolute_time(),
		.level = gpio_get_level(PIN) > 0,
	};
    xQueueSendFromISR(gpio_evt_queue, &w, NULL);
}

static void gpio_task_example(void* arg)
{
	Wave w;
    for(;;) {
        if(xQueueReceive(gpio_evt_queue, &w, portMAX_DELAY)) {
            // printf("GPIO[%u] intr, val: %d\n", io_num, gpio_get_level(io_num));
			gpio_callback(&w);
        }
    }
}

extern "C" void app_main() {
	printf("Hello GPIO IRQ\n");

	gpio_reset_pin(PIN);
	

    //create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(100, sizeof(Wave));
    //start gpio task
    xTaskCreate(gpio_task_example, "gpio_task_example", 2048, NULL, 10, NULL);
	
	gpio_set_direction(PIN, GPIO_MODE_INPUT);
	if (sgDecoders[0]->highFirst()) {
		gpio_set_pull_mode(PIN, GPIO_PULLDOWN_ONLY);
	} else {
		gpio_set_pull_mode(PIN, GPIO_PULLUP_ONLY);
	}
	gpio_set_intr_type(PIN, GPIO_INTR_ANYEDGE);
	// 参数是用来控制中断的优先级的，文档的注释里面说 C 语言需要低优先级。
	gpio_install_isr_service(ESP_INTR_FLAG_LOWMED);
	gpio_isr_handler_add(PIN, gpio_isr_handler, NULL);

	printf("here\n");
	
	gpio_reset_pin(GPIO_NUM_18);
	gpio_set_direction(GPIO_NUM_18, GPIO_MODE_OUTPUT);
	gpio_set_pull_mode(GPIO_NUM_18, GPIO_PULLUP_ONLY);
	
	bool t = false;

	// Wait forever
	while (1) {
		// printf("sleeping\n");
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		gpio_set_level(GPIO_NUM_18, t ? 1 : 0);
		t = !t;
	}
}

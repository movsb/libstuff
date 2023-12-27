#pragma once

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

namespace ebp {
namespace modules {
namespace displays {
namespace ssd1306 {

class SSD1306 {
public:
	SSD1306(uint8_t width, uint8_t height)
		: _width(width)
		, _height(height)
		  {}
public:
	void init();
	void render();

public:
	void writeChar(uint8_t x, uint8_t y, uint8_t ch);
	void writeString(uint8_t x, uint8_t y, const char *str);

protected:
	void _i2cWrite(const uint8_t *data, uint16_t len);
	void _writeCommand(uint8_t cmd);

protected:
	uint8_t _width;
	uint8_t _height;
	uint8_t *_fb;
};

#define SSD1306_I2C_ADDR            uint8_t(0x3C)


struct render_area {
	uint8_t start_col;
	uint8_t end_col;
	uint8_t start_page;
	uint8_t end_page;

	int buflen;
};

typedef void (*i2c_write_t)(uint8_t *buf, int len);
extern i2c_write_t i2c_write;

inline void calc_render_area_buflen(struct render_area *area) {
	// calculate how long the flattened buffer will be for a render area
	area->buflen = (area->end_col - area->start_col + 1) * (area->end_page - area->start_page + 1);
}

inline void SSD1306_send_cmd(uint8_t cmd) {
	// I2C write process expects a control byte followed by data
	// this "data" can be a command or data to follow up a command
	// Co = 1, D/C = 0 => the driver expects a command
	uint8_t buf[2] = {0x80, cmd};
	i2c_write(buf, 2);
}

}
}
}
}

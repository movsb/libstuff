#include "ssd1306.hpp"
#include "font.hpp"

namespace ebp {
namespace modules {
namespace displays {
namespace ssd1306 {

struct Command {
	enum Value {
		// 基础命令
		SetContrastControl          = 0x81, // 设置对比度，256 阶，双字节命令
		EntireDisplayOn             = 0xA4, // 整个显示打开。（+0: 按显存显示，+1: 全部点亮）
		SetNormalDisplay            = 0xA6, // 正常显示模式，1 亮，0 灭
		SetInverseDisplay           = 0xA7, // 反转显示模式，0 亮，1 灭
		SetDisplayOff               = 0xAE, // 关闭显示（睡眠模式）
		SetDisplayOn                = 0xAF, // 正常显示

		// 滚动命令

		// 显存地址设置命令
		
	};
};

#define SSD1306_SET_MEM_MODE        (uint8_t)(0x20)
#define SSD1306_SET_COL_ADDR        (uint8_t)(0x21)
#define SSD1306_SET_PAGE_ADDR       (uint8_t)(0x22)
#define SSD1306_SET_HORIZ_SCROLL    (uint8_t)(0x26)
// #define SSD1306_SET_VERT_SCROLL     (uint8_t)(0x29)
#define SSD1306_SET_SCROLL          (uint8_t)(0x2E)

#define SSD1306_SET_DISP_START_LINE (uint8_t)(0x40)

#define SSD1306_SET_CHARGE_PUMP     (uint8_t)(0x8D)

#define SSD1306_SET_SEG_REMAP       (uint8_t)(0xA0)
#define SSD1306_SET_ENTIRE_ON       (uint8_t)(0xA4)
#define SSD1306_SET_ALL_ON          (uint8_t)(0xA5)
#define SSD1306_SET_NORM_DISP       (uint8_t)(0xA6)
#define SSD1306_SET_INV_DISP        (uint8_t)(0xA7)
#define SSD1306_SET_MUX_RATIO       (uint8_t)(0xA8)
#define SSD1306_SET_DISP            (uint8_t)(0xAE)
#define SSD1306_SET_COM_OUT_DIR     (uint8_t)(0xC0)
#define SSD1306_SET_COM_OUT_DIR_FLIP (uint8_t)(0xC0)

#define SSD1306_SET_DISP_OFFSET     (uint8_t)(0xD3)
#define SSD1306_SET_DISP_CLK_DIV    (uint8_t)(0xD5)
#define SSD1306_SET_PRECHARGE       (uint8_t)(0xD9)
#define SSD1306_SET_COM_PIN_CFG     (uint8_t)(0xDA)
#define SSD1306_SET_VCOM_DESEL      (uint8_t)(0xDB)

#define SSD1306_PAGE_HEIGHT         (uint8_t)(8)
#define SSD1306_NUM_PAGES           (64 / SSD1306_PAGE_HEIGHT)
#define SSD1306_BUF_LEN             (SSD1306_NUM_PAGES * 128)

#define SSD1306_WRITE_MODE         (uint8_t)(0xFE)
#define SSD1306_READ_MODE          (uint8_t)(0xFF)

render_area *area = new render_area();
i2c_write_t i2c_write;
void SSD1306_send_cmd_list(uint8_t *buf, int num);

void SSD1306_send_cmd_list(uint8_t *buf, int num) {
	for (int i=0;i<num;i++)
		SSD1306_send_cmd(buf[i]);
}

void SSD1306_send_buf(uint8_t buf[], int buflen) {
	// in horizontal addressing mode, the column address pointer auto-increments
	// and then wraps around to the next page, so we can send the entire frame
	// buffer in one gooooooo!

	// copy our frame buffer into a new buffer because we need to add the control byte
	// to the beginning

	uint8_t *temp_buf = new uint8_t[buflen + 1];

	temp_buf[0] = 0x40;
	memcpy(temp_buf+1, buf, buflen);

	i2c_write(temp_buf, buflen+1);

	delete (temp_buf);
}

// § 8.5 Reset Circuit
//
// 官方默认的复位顺序。
//
// 1. Display is OFF
// 2. 128 x 64 Display Mode
// 3. Normal segment and display data column address and row address mapping (SEG0 mapped to address 00h and COM0 mapped to address 00h)
// 4. Shift register data clear in serial interface
// 5. Display start line is set at display RAM address 0
// 6. Column address counter is set at 0
// 7. Normal scan direction of the COM outputs
// 8. Contrast control register is set at 7Fh
// 9. Normal display mode (Equivalent to A4h command)
void SSD1306::init() {
	uint8_t cmds[] = {
		SSD1306_SET_DISP,               // set display off
		/* memory mapping */
		SSD1306_SET_MEM_MODE,           // set memory address mode 0 = horizontal, 1 = vertical, 2 = page
		0x00,                           // horizontal addressing mode
		/* resolution and layout */
		SSD1306_SET_DISP_START_LINE,    // set display start line to 0
		SSD1306_SET_SEG_REMAP | 0x01,   // set segment re-map, column address 127 is mapped to SEG0
		SSD1306_SET_MUX_RATIO,          // set multiplex ratio
		_height - 1,             // Display height - 1
		SSD1306_SET_COM_OUT_DIR | 0x08, // set COM (common) output scan direction. Scan from bottom up, COM[N-1] to COM0
		SSD1306_SET_DISP_OFFSET,        // set display offset
		0x00,                           // no offset
		SSD1306_SET_COM_PIN_CFG,        // set COM (common) pins hardware configuration. Board specific magic number. 
										// 0x02 Works for 128x32, 0x12 Possibly works for 128x64. Other options 0x22, 0x32
		0x12,

		/* timing and driving scheme */
		SSD1306_SET_DISP_CLK_DIV,       // set display clock divide ratio
		0x80,                           // div ratio of 1, standard freq
		SSD1306_SET_PRECHARGE,          // set pre-charge period
		0xF1,                           // Vcc internally generated on our board
		SSD1306_SET_VCOM_DESEL,         // set VCOMH deselect level
		0x30,                           // 0.83xVcc
		/* display */
		SSD1306_SET_CONTRAST,           // set contrast control
		0xFF,
		SSD1306_SET_ENTIRE_ON,          // set entire display on to follow RAM content
		SSD1306_SET_NORM_DISP,           // set normal (not inverted) display
		SSD1306_SET_CHARGE_PUMP,        // set charge pump
		0x14,                           // Vcc internally generated on our board
		SSD1306_SET_SCROLL | 0x00,      // deactivate horizontal scrolling if set. This is necessary as memory writes will corrupt if scrolling was enabled
		SSD1306_SET_DISP | 0x01, // turn display on
	};

	SSD1306_send_cmd_list(cmds, sizeof(cmds) / sizeof(cmds[0]));
	//
	// Initialize render area for entire frame (SSD1306_WIDTH pixels by SSD1306_NUM_PAGES pages)
	struct render_area frame_area = {
		start_col: 0,
		end_col : 128 - 1,
		start_page : 0,
		end_page : SSD1306_NUM_PAGES - 1
		};
	area->start_col = 0,
	area->end_col = 127;
	area->start_page = 0,
	area->end_page = SSD1306_NUM_PAGES - 1;
	calc_render_area_buflen(area);

    // SSD1306_send_cmd(SSD1306_SET_INV_DISP);
    SSD1306_send_cmd(SSD1306_SET_NORM_DISP);

	// zero the entire display
	uint8_t *buf = new uint8_t[area->buflen];
	memset(buf, 0, area->buflen);
	_fb = buf;

	char *text[] = {
		"A long time ago",
		"  on an OLED ",
		"   display",
		" far far away",
		"Lived a small",
		"red raspberry",
		"by the name of",
		"    PICO"
	};

	int y = 0;
	for (int i = 0 ;i < sizeof(text)/sizeof(text[0]); i++) {
		writeString(5, y, text[i]);
		printf("str: %s\n", text[i]);
		y+=8;
	}
	render();
	
	printf("zeroed\n");
}

void SSD1306::_i2cWrite(const uint8_t *data, uint16_t len) {

}

// § 8.1.5.1 I2C-bus Write data
// 控制字节由 Co 和 D/C# 后跟 6 个 0 构成。
// 如果 Co 为 0，则表示只传输数据字节，为 1 则为命令。
// D/C#：表示接下来的字节是命令还是数据。
void SSD1306::_writeCommand(uint8_t cmd) {
	uint8_t buf[2] = {0x80, cmd};
	_i2cWrite(&buf[0], 2);
}

void SSD1306::render() {
	uint8_t cmds[] = {
		SSD1306_SET_COL_ADDR,
		area->start_col,
		area->end_col,
		SSD1306_SET_PAGE_ADDR,
		area->start_page,
		area->end_page
	};
	
	SSD1306_send_cmd_list(cmds, sizeof(cmds)/sizeof(cmds[0]));
	SSD1306_send_buf(_fb, area->buflen);
}

static inline int GetFontIndex(uint8_t ch) {
	if (ch >= 'A' && ch <='Z') {
		return  ch - 'A' + 1;
	}
	else if (ch >= '0' && ch <='9') {
		return  ch - '0' + 27;
	}
	else return  0; // Not got that char so space.
}


static uint8_t reverse(uint8_t b) {
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}
void SSD1306::writeChar(uint8_t x, uint8_t y, uint8_t ch) {
	if (x > _width - 8 || y > _height - 8) {
		return;
	}
	y = y/8;

	ch = toupper(ch);
	int idx = GetFontIndex(ch);
	int fb_idx = y * 128 + x;

	for (int i=0;i<8;i++) {
		_fb[fb_idx++] = reverse(_ascii_font[idx * 8 + i]);
	}
}

void SSD1306::writeString(uint8_t x, uint8_t y, const char *str) {
	if (x > _width - 8 || y > _height - 8) {
		return;
	}
	
	while(*str) {
		writeChar(x, y, *str);
		str++;
		x += 8;
	}
}
	
}
}
}
}

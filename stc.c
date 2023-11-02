#include <stdio.h>

#define __STC_STC8G1K08A__
#define FOSC 11059200UL
#include "stc.h"

volatile int8 uartBusy;

void UARTInit(uint32 baudrate) {
	uint16 t = 65536 - FOSC / baudrate / 4;
	SCON = 0x50;
	TL1 = (byte)(t);
	TH1 = (byte)(t >> 8);
	AUXR = 0x40;
	TR1 = 1;
	ES = 1;
	uartBusy = 0;
}

void UARTSendByte(uint8 c) {
	while(uartBusy);
	uartBusy = 1;
	SBUF = c;
}

void UARTSendData(const uint8 *data, uint8 len) {
	while(len-- > 0) {
		UARTSendByte(*data);
		++data;
	}
}

void UARTSendString(const int8 *str) {
	while(*str) {
		UARTSendByte(*str);
		++str;
	}
}

static void _UartOutputChar(char c, void *p) {
	(p);
	UARTSendByte(c);
}

void UARTSendFormat(const int8 *format, ...) {
	va_list ap;
	va_start(ap, format);
	_print_format(_UartOutputChar, NULL, format, ap);
	va_end(ap);
}

void UARTHandler(void) INTERRUPT(4) {
	if(TI) {
		TI = 0;
		uartBusy = 0;
	}
	if (RI) {
		RI = 0;
		// read SBUF;
	}
}

#define CLK(n) (P32 = (n))
#define DAT(n) (P33 = (n))
#define DAT_GET() (P33)
#define CSN(n)  (P54 = (n))

static void _SpiWriteByte(uint8 c);
static uint8 _SpiReadByte(void);

// SPI 函数
void SpiInit(void) {

}

static void _SpiWriteByte(uint8 c) {
	CLK(0);
	
	for(int8 i=0; i<8; i++) {
		CLK(0);
		DAT(c & 0x80 ? 1 : 0);
		CLK(1);
		c <<= 1;
	}

	CLK(0);
}

static uint8 _SpiReadByte(void) {
	uint8 d = 0;
	CLK(0);
	for(int8 i=0; i<8; i++) {
		d <<= 1;
		CLK(1);
		if (DAT_GET() > 0) {
			d |= 1;
			UARTSendFormat("读取到高电平！\r\n");
		} else {
			// UARTSendFormat("读取到低电平！\r\n");
		}
		CLK(0);
	}
	CLK(0);
	return d;
}

void SpiWrite(uint8 cmd, uint8 value) {
	CSN(0);
	_SpiWriteByte(cmd);
	_SpiWriteByte(value);
	CSN(1);
}

void SpiWrites(uint8 cmd, const uint8 *data, uint8 len) {
	CSN(0);
	_SpiWriteByte(cmd);
	while(len > 0) {
		_SpiWriteByte(*data);
		++data;
		--len;
	}
	CSN(1);
}

uint8 SpiRead(uint8 cmd) {
	uint8 value;
	CSN(0);
	_SpiWriteByte(cmd);
	// CSN(1);
	// CSN(0); // "并且 CSN 拉低后，DATA 切换成输出口"
	value = _SpiReadByte();
	CSN(1);
	// 写之前会拉低，这儿没必要拉
	// CSN(0); // "当CSN 再次拉低之后，DATA 切换为输入口"
	return value;
}

void SpiReads(uint8 cmd, uint8 *data, uint8 len) {
	CSN(0);
	_SpiWriteByte(cmd);
	// CSN(1);
	// CSN(0);
	while(len > 0) {
		*data = _SpiReadByte();
		++data;
		--len;
	}
	CSN(1);
}

// 电源控制（Power Control，Pc）
// PowerControl_EnableWakeupTimer

// 使用stc15掉电唤醒专用定时器和掉电模式时有哪三个容易犯错的问题
// http://www.dumenmen.com/thread-1078-1-1.html

// 启用唤醒定时器。
// 单位精确到秒（够用？）暂定最多 10 秒（因为 32K 的频率寄存器计数不正确）
void PowerControl_EnableWakeupTimer(int16 seconds) {
	if(seconds < 1) {
		return;
	}

	// Target wakeup frequency: 34.900 KHz
	// TODO: 应该用 PowerControl_GetWakeupTimerClockFrequency() 获取，
	// 但是目前拿到的数据是错误的。
	static const uint32 wakeUpFrequency = 34900;

	uint16 count = (uint32)(seconds) * wakeUpFrequency / 16;

	WKTCL = (uint8)(count&0xFF);
	WKTCH = (uint8)(count >> 8) | 0x80; // 只能赋值，不能 或 运算
}

void PowerControl_DisableWakeupTimer(void) {
	WKTCH = 0x00; // 不要用位运算
}

// 获取内部掉电唤醒专用定时器出厂时所记录的时钟频率
// 官方文档的 0xF8 和 0xF9 是错误的，“7.3.6 读取 32K 掉电唤醒定时器的频率 (从 RAM 中读取)” 已经被划掉了。
/* 拿到的数据是错的，先屏蔽。
uint16 PowerControl_GetWakeupTimerClockFrequency(void) {
	uint16 hi = *(int8 __code*)0x1FF5;
	uint16 lo = *(int8 __code*)0x1FF6;
	return hi << 8 | lo;
}
*/

void PowerControl_PowerDown(void) {
	while(1) {
		PCON |= PD;
		NOP();
		NOP();

		// TODO: 长时间唤醒定时器

		break;
	}
}

// 7.3 存储器中的特殊参数
#ifdef __STC_STC8G1K08A__
	#define STC_UUID_ADDR   0x1FF9 // 7 个字节
#endif


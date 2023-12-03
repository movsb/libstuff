#include <stdio.h>
#include <stdint.h>

#define __STC_STC8G1K08A__
#define FOSC 11059200UL
#include "stc.h"

static volatile int8_t uartBusy;

void UARTInit(uint32_t baudrate) {
	uint16_t t = 65536 - FOSC / baudrate / 4;
	SCON = 0x50;
	TL1 = (uint8_t)(t);
	TH1 = (uint8_t)(t >> 8);
	AUXR = 0x40;
	TR1 = 1;
	ES = 1;
	uartBusy = 0;
}

void UARTSendByte(uint8_t c) {
	while(uartBusy);
	uartBusy = 1;
	SBUF = c;
}

void UARTSendData(const uint8_t *data, uint8_t len) {
	while(len-- > 0) {
		UARTSendByte(*data);
		++data;
	}
}

void UARTSendString(const int8_t *str) {
	while(*str) {
		UARTSendByte(*str);
		++str;
	}
}

static void _UartOutputChar(char c, void *p) {
	(p);
	UARTSendByte(c);
}

void UARTSendFormat(const int8_t *format, ...) {
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

// 电源控制（Power Control，Pc）
// PowerControl_EnableWakeupTimer

// 使用stc15掉电唤醒专用定时器和掉电模式时有哪三个容易犯错的问题
// http://www.dumenmen.com/thread-1078-1-1.html
// 请问关于STC8G掉电唤醒定时器问题
// https://www.amobbs.com/forum.php?mod=viewthread&action=printable&tid=5744553

// 启用唤醒定时器。
// 单位精确到秒（够用？）暂定最多 10 秒（因为 32K 的频率寄存器计数不正确）
void PowerControl_EnableWakeupTimer(int16_t seconds) {
	if(seconds < 1) {
		return;
	}

	// Target wakeup frequency: 34.900 KHz
	// TODO: 应该用 PowerControl_GetWakeupTimerClockFrequency() 获取，
	// 但是目前拿到的数据是错误的。
	static const uint32_t wakeUpFrequency = 34900;

	uint16_t count = (uint32_t)(seconds) * wakeUpFrequency / 16;
	if(count >= 32766) {
		count = 32766;
	}

	WKTCL = (uint8_t)(count&0xFF);
	WKTCH = (uint8_t)(count >> 8) | 0x80; // 只能赋值，不能 或 运算
}

void PowerControl_DisableWakeupTimer(void) {
	WKTCH = 0x00; // 不要用位运算
}

// 获取内部掉电唤醒专用定时器出厂时所记录的时钟频率
// 官方文档的 0xF8 和 0xF9 是错误的，“7.3.6 读取 32K 掉电唤醒定时器的频率 (从 RAM 中读取)” 已经被划掉了。
// 目前计数是错误的，不要用。
uint16_t PowerControl_GetWakeupTimerClockFrequency(void) {
	uint16_t hi = *(int8_t __CODE*)0x1FF5;
	uint16_t lo = *(int8_t __CODE*)0x1FF6;
	return hi << 8 | lo;
}

void PowerControl_PowerDown(void) {
	while(1) {
		PCON |= PD;
		NOP();
		NOP();

		// TODO: 长时间唤醒定时器

		break;
	}
}

void PowerControl_Idle(void) {
	PCON |= IDL;
}

void PowerControl_SoftReset(void) {
	IAP_CONTR |= SWRST;
}

void PowerControl_SoftResetToIap(void) {
	IAP_CONTR |= SWBS | SWRST;
}

void PowerControl_EnableWatchDogTimer(uint8_t wdtPS) {
	WDT_CONTR = EN_WDT | (wdtPS & WDT_PS);
}

void PowerControl_ClearWatchDogTimer(void) {
	WDT_CONTR |= CLR_WDT;
}

// 7.3 存储器中的特殊参数
#ifdef __STC_STC8G1K08A__
	#define STC_GUID_ADDR   0x1FF9 // 7 个字节
#endif

const uint8_t* Flash_GetGuid(void) {
	return (const uint8_t __CODE*)(STC_GUID_ADDR);
}

// 用线性同余随机生成的随机数。
// 系数是任意选的
static uint8_t seed_inited = 0;
static uint16_t seed;
void GenRandom(uint8_t *buf, uint8_t len) {
	if (!seed_inited) {
		const uint8_t *guid = Flash_GetGuid();
		for (uint8_t i = 0; i < 6; i+=2) {
			seed ^= *(uint16_t*)&guid[i];
		}
		seed ^= guid[6];
		seed_inited = 1;
		// UARTSendFormat("GUID: %02X %02X %02X\r\n", guid[0], guid[1], guid[2]);
	}

	for (uint8_t i = 0; i < len; i++) {
		seed *= 31;
		seed += 71;
		buf[i] = seed % 255;
	}
}

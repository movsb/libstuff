#ifndef __STC_H__
#define __STC_H__

#include <stdint.h>

#ifndef __SDCC
#define __SDCC
#endif

// TODO
#define FOSC 11059200UL

#ifndef FOSC
	#error "FOSC is not defined"
#endif

// 一些基础类型
// TODO：类型大小检测符合位数。
typedef signed char         int8;
typedef signed int          int16;
typedef signed long         int32;

typedef unsigned char       byte;
typedef unsigned char       uint8;
typedef unsigned int        uint16;
typedef unsigned long       uint32;

// 适用于 SDCC - Small Device C Compiler 的宏定义
#if defined (SDCC) || defined (__SDCC)

#define SBIT(name, addr, bit)       __sbit  __at(addr+bit)  name
#define SFR(name, addr)             __sfr   __at(addr)      name
#define INTERRUPT(vector)           __interrupt (vector)

#define NOP() __asm NOP __endasm

#endif // SDCC

// 特殊功能寄存器
//
// 注意：寄存器地址能够被 8 整除的才可进行位寻址，不能被 8 整除的则不可位寻址。

// 电源控制寄存器（复位值：0011,0000）
SFR(PCON,   0x87);
	#define IDL     0x01    // 空闲模式控制位。1：单片机进入 IDLE 模式，只有 CPU 停止工作，其他外设依然在运行。唤醒后硬件自动清零。
	#define PD      0x02    // 时钟停振模式/掉电模式/停电模式控制位（Power Down）。1：单片机进入时钟停振模式/掉电模式/停电模式，CPU 以及全部外设均停止工作。唤醒后硬件自动清零。
	#define GF0     0x04
	#define GF1     0x08
	#define POF     0x10    // 上电复位标志位。MCU 每次重新上电后，硬件自动将此位置 1，可软件将此位清零。
	#define LVDF    0x20    // 低压检测标志位。当系统检测到低压事件时，硬件自动将此位置 1，并向 CPU 提出中断请求。此位需要用户软件清零。
	#define SMOD0   0x40
	#define SMOD    0x80

SFR(TCON,   0x88);
	SBIT(TR1,   0x88,   6);
	SBIT(IT1,   0x88,   2); // 外部中断源1触发控制位。
SFR(TMOD,   0x89);
SFR(TL1,    0x8B);
SFR(TH1,    0x8D);

SFR(AUXR,   0x8E);
SFR(P1M1,   0x91);
SFR(P1M0,   0x92);
SFR(P0M1,   0x93);
SFR(P0M0,   0x94);
SFR(P2M1,   0x95);
SFR(P2M0,   0x96);
SFR(SCON,   0x98);
	SBIT(RI,    0x98, 0);
	SBIT(TI,    0x98, 1);
SFR(SBUF,   0x99);

SFR(IE,     0xA8);
	SBIT(EX1,   0xA8,   2);
	SBIT(ET1,   0xA8,   3);
	SBIT(ES,    0xA8,   4);
	SBIT(EA,    0xA8,   7);
	
SFR(P3,     0xB0);
	SBIT(P30,   0xB0,   0);
	SBIT(P31,   0xB0,   1);
	SBIT(P32,   0xB0,   2);
	SBIT(P33,   0xB0,   3);
	
SFR(P3M1,   0xB1);
SFR(P3M0,   0xB2);
SFR(P4M1,   0xB3);
SFR(P4M0,   0xB4);

SFR(WDT_CONTR,      0xC1);
#define WDT_FLAG    0x80    // 看门狗溢出标志，看门狗发生溢出时，硬件自动将此位置 1，需要软件清零。
#define EN_WDT      0x20    // 1：启动看门狗定时器。
#define CLR_WDT     0x10    // 1：清零看门狗定时器，自动复位
#define WDT_PS      0x07    // 分频系数

SFR(IAP_CONTR,      0xC7);
	#define SWBS    0x40 // 复位后进入 IAP
	#define SWRST   0x20 // 软件复位

SFR(P5,     0xC8);
	SBIT(P54,   0xC8,   4);
	SBIT(P55,   0xC8,   5);

SFR(P5M1,   0xC9);
SFR(P5M0,   0xCA);
SFR(P6M1,   0xCB);
SFR(P6M0,   0xCC);

SFR(T2H,    0xD6);
SFR(T2L,    0xD7);

// 全局中断相关函数
inline void EnableInterrupts(void) {
	EA = 1;
}

inline void DisableInterrupts(void) {
	EA = 0;
}

void UARTInit(uint32 baudrate);
void UARTSendByte(uint8 byte);
void UARTSendData(const uint8 *data, uint8 len);
void UARTSendString(const int8 *str);
void UARTSendFormat(const int8 *format, ...);

// 中断函数必须要在头文件中声明，否则中断函数不会被执行
// https://blog.csdn.net/openblog/article/details/72942315
void UARTHandler(void) INTERRUPT(4);

// SPI
// 命令和寄存器并不一样，SPI 第一个字节是命令，而命令分读、写寄存器、清空发送数据等。
// 所以不要认为 spi 读写的都是寄存器。
void    SpiInit(void);
void    SpiWrite        (uint8 cmd, uint8 value);
void    SpiWrites       (uint8 cmd, const uint8 *data, uint8 len);
uint8   SpiRead         (uint8 cmd);
void    SpiReads        (uint8 cmd, uint8 *data, uint8 len);

void SpiSetDataBit(uint8 bit);
uint8 SpiGetDataBit(void);

// 电源控制。
// 
// 

// 掉电唤醒
// 如果 STC8 系列单片机内置掉电唤醒专用定时器被允许（通过软件将 WKTCH 寄存器中的 WKTEN
// 位置 1），当 MCU 进入掉电模式/停机模式后，掉电唤醒专用定时器开始计数，当计数值与用户所设置的
// 值相等时，掉电唤醒专用定时器将 MCU 唤醒。

SFR(WKTCL,      0xAA);  // 掉电唤醒定时器计数寄存器
SFR(WKTCH,      0xAB);
	#define WKTEN       0x80

void PowerControl_EnableWakeupTimer(int16 seconds);
void PowerControl_DisableWakeupTimer(void);
uint16 PowerControl_GetWakeupTimerClockFrequency(void);

// 进入掉电模式。
void PowerControl_PowerDown(void);
// 复位
void PowerControl_SoftReset(void);
// 复位到下载模式
void PowerControl_SoftResetToIap(void);
// 启动看门狗定时器
void PowerControl_EnableWatchDogTimer(uint8_t wdtPS);
// 访问（清空）看门狗定时器
void PowerControl_ClearWatchDogTimer(void);

// 7.3 存储器中的特殊参数
// 全球唯一 ID 号 
const uint8_t* Flash_GetGuid(void);

// 使用芯片全球唯一 ID 生成的随机数
void GenRandom(uint8_t *buf, uint8_t len);


#endif // __STC_H__

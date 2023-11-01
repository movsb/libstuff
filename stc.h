#ifndef __STC_H__
#define __STC_H__

#ifndef __SDCC
#define __SDCC
#endif

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

#endif // SDCC

// 特殊功能寄存器
SFR(TCON,   0x88);
	SBIT(TR1,   0x88,   6);
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

SFR(P5,     0xC8);
	SBIT(P54,   0xC8,   4);

SFR(P5M1,   0xC9);
SFR(P5M0,   0xCA);
SFR(P6M1,   0xCB);
SFR(P6M0,   0xCC);

SFR(P3M1,   0xB1);
SFR(P3M0,   0xB2);
SFR(P5M1,   0xC9);
SFR(P5M0,   0xCA);
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

#endif // __STC_H__

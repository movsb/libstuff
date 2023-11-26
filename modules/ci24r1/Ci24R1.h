#pragma once

#include <stdint.h>

#define R_RX_PL_WID         0x60
#define R_RX_PAYLOAD        0x61
#define CE_ON               0x70
#define CE_OFF              0x71
#define SELSPI              0x74 // 选择 DATA 引脚为 SPI 功能
#define SELIRQ              0x75 // 选择 DATA 引脚为输出 IRQ 值
#define W_TX_PAYLOAD        0xA0 // 写发射负载数据
#define W_TX_PAYLOAD_NOACK  0xB0
#define FLUSH_TX            0xE1
#define FLUSH_RX            0xE2

#define R_REGISTER(n) (0x00 + (n))
#define W_REGISTER(n) (0x20 + (n))

#define CONFIG          0x00
	#define PRIM_RX     0x01    // 发射/接收配置，只能在Shutdown和Standby下更改。0：发射模式 1：接收模式
	#define PWR_UP      0x02    // 关断/开机模式配置. 0：关断模式 1：开机模式
	#define CRCO        0x04    // CRC长度配置 0：1 字节，1: 2字节
	#define EN_CRC      0x08    // 启用 CRC 校验。
	
#define EN_AA           0x01
	#define REG0F_SEL_L 0xC0
	#define ENAA_P0     0x01
	#define REG0F_SEL_L_SET(n)  ((n##u & 0x03) << 6)

#define EN_RXADDR       0x02
	#define REG0F_SEL_H 0xC0
	#define ERX_P0      0x01
	#define REG0F_SEL_H_SET(n)  ((n##u & 0x0C) << 6)

#define SETUP_AW        0x03

#define SETUP_RETR      0x04    // 自动重发配置
	#define ARD         0xF0    // 自动重发延时配置
	#define ARC         0x0F    // 最大自动重发次数
								// 自动重发延时：[250us, 500us, ..., 4000us]
	#define ARD_SET(us)         (((us##u / 250 - 1) & 0x0F) << 4)
								// 最大自动重发次数
	#define ARC_SET(times)      (times##u & 0x0F)

#define RF_CH           0x05
#define RF_SETUP        0x06
	#define RF_DR_LOW   0x20
	#define RF_DR_HIGH  0x04
	#define RF_DR_SET(n)        ((n & 0x02 ? RF_DR_HIGH : 0) | (n & 0x01 ? RF_DR_LOW : 0))
	#define RF_PWR_SET(n)       (n & 0x07)
#define STATUS          0x07
	#define TX_FULL     0x01        // TX FIFO满标志位。
	#define RX_P_NO_MASK    0x0E    // 收到数据的接收管道PPP号。
	#define RX_P_NO_GET(status)     ((status & RX_P_NO_MASK) >> 1)
	#define MAX_RT      0x10        // 达到最大重发次数中断位，写’1’清除。
	#define TX_DS       0x20        // 发射端发射完成中断位，如果是ACK模式，则收到ACK确认信号后TX_DS 位置’1’，写’1’清除。
	#define RX_DR       0x40        // RX FIFO有值标志位，写’1’清除。
#define RX_ADDR_P0      0x0A
#define RX_ADDR_P2      0x0C
#define OSC_CAP         0x0F        // 0F_2
	#define OSC_CAP_SET(n)      (n##u << 4)
#define TX_ADDR         0x10
#define RX_PW_P0        0x11
#define FIFO_STATUS     0x17
#define FEATURE         0x1D
	#define EN_DYN_ACK  0x01    // 使能命令W_TX_PAYLOAD_NOACK
	#define EN_DPL      0x04    // 使能动态负载长度

void Ci24R1_TX_Mode(void);
uint8_t Ci24R1_TxPacket(void);

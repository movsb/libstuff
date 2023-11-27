#include "Ci24R1.h"
#include <spi/spi.h>

static const uint8_t __code addr[5] = {1,2,3,4,5};

static void Ci24R1_TX_Mode(ci24r1_config_t *c);
static void Ci24R1_RX_Mode(ci24r1_config_t *c);

// TODO 删除
extern void UARTSendString(const int8_t *str);
extern void UARTSendFormat(const int8_t *format, ...);

uint8_t ci24r1_online(ci24r1_config_t *c) {
	ci24r1_sel_spi(c);
	uint8_t status = spi_read(c->spi, R_REGISTER(STATUS));
	// TODO 使用 0 和 1 共同判断，而不止是最高位
	return (status & 0x80) == 0;
}

void ci24r1_mode(ci24r1_config_t *c, ci24r1_mode_t mode) {
	ci24r1_sel_spi(c);

	switch (mode) {
		case CI24R1_MODE_TX:
			return Ci24R1_TX_Mode(c);
		case CI24R1_MODE_RX:
			return Ci24R1_RX_Mode(c);
		default:
			return;
	}
}

void Ci24R1_TX_Mode(ci24r1_config_t *c)
{
	spi_config_t *spi = c->spi;
	spi_write(spi, CE_OFF,0x00);
	spi_write(spi, W_REGISTER(SETUP_AW),            0x03);
	spi_write_multi(spi, W_REGISTER(TX_ADDR),      (uint8_t *)addr, 5);
	spi_write_multi(spi, W_REGISTER(RX_ADDR_P0),   (uint8_t*)addr,5);
	spi_write(spi, W_REGISTER(EN_RXADDR),     ERX_P0 | REG0F_SEL_H_SET(2));
	spi_write(spi, W_REGISTER(EN_AA),         ENAA_P0 | REG0F_SEL_L_SET(2));
	spi_write(spi, W_REGISTER(OSC_CAP),       OSC_CAP_SET(0b1011));
	spi_write(spi, W_REGISTER(FEATURE),       0);
	spi_write(spi, W_REGISTER(RF_CH),         80);
	spi_write(spi, W_REGISTER(RF_SETUP),      RF_DR_SET(2) | RF_PWR_SET(0));
	spi_write(spi, W_REGISTER(SETUP_RETR),    ARD_SET(500) | ARC_SET(15));
	spi_write(spi, W_REGISTER(CONFIG),        PWR_UP | EN_CRC | CRCO);
	spi_write(spi, CE_ON, 0x00);
}

uint8_t ci24r1_send(ci24r1_config_t *c, const uint8_t *data, uint8_t len) {
	spi_config_t *spi = c->spi;

	uint8_t *data2 = (uint8_t*)data;
	// TODO remove
	if (++data2[3]== 0) {
		if (++data2[2] == 0) {
			if (++data2[1] == 0) {
				++data2[0];
			}
		}
	}

	spi_write(spi, CE_OFF,0x00);
	spi_write(spi, FLUSH_TX,0x00);
	spi_write_multi(spi, W_TX_PAYLOAD ,data, len); //写 TX FIFO
	spi_write(spi, CE_ON,0x00); //CE 拉高，Ci24R1 开始发射 HAL_Delay(10); //等待数据发送完成
						  //
	while(1) {
		uint8_t status=spi_read(spi, R_REGISTER(STATUS));
		if ((status & 0x80) != 0) {
			UARTSendFormat("状态寄存器读数错误：%d\r\n", status);
			return 255;
		}
		if (status & TX_FULL) {
			UARTSendFormat("发送数据满了: status=%d\r\n", status);
			continue;
		} else if(status & MAX_RT) {
			UARTSendFormat("达到最大重发次数\r\n");
			// § 4.2.1 ACK 模式
			// MAX_RT 中断在清除之前不能进行下一步的数据发送
			spi_write(spi, W_REGISTER(STATUS), status | MAX_RT);
			break;
		} else if(status & TX_DS) {
			UARTSendFormat("数据发送完成: %02X %02X %02X %02X\r\n", data[0], data[1], data[2], data[3]);
			spi_write(spi, W_REGISTER(STATUS), status);
			break;
		} else {
			UARTSendFormat("循环发送中: %d\r\n", status);
		}
	}
	
	// UARTSendString("退出发送\r\n");
	return 0xff;
}

void Ci24R1_RX_Mode(ci24r1_config_t *c) {
	spi_config_t *spi = c->spi;
	spi_write(spi, CE_OFF,0x00);
	spi_write(spi, W_REGISTER(SETUP_AW),      0x03);
	spi_write_multi(spi, W_REGISTER(RX_ADDR_P0),   (uint8_t*)addr,5);
	spi_write(spi, W_REGISTER(RX_PW_P0),      4);
	spi_write(spi, W_REGISTER(EN_RXADDR),     ERX_P0 | REG0F_SEL_H_SET(2));
	spi_write(spi, W_REGISTER(EN_AA),         ENAA_P0 | REG0F_SEL_L_SET(2));
	spi_write(spi, W_REGISTER(OSC_CAP),       OSC_CAP_SET(0b1011));
	spi_write(spi, W_REGISTER(FEATURE),       EN_DYN_ACK);
	spi_write(spi, W_REGISTER(RF_CH),         80);
	spi_write(spi, W_REGISTER(RF_SETUP),      RF_DR_SET(2) | RF_PWR_SET(0));
	spi_write(spi, W_REGISTER(CONFIG),        (PWR_UP | EN_CRC | CRCO | PRIM_RX));
	spi_write(spi, CE_ON,0x00);
}

uint8_t ci24r1_recv(ci24r1_config_t *c, uint8_t *data, uint8_t *len) {
	uint8_t status = spi_read(c->spi, R_REGISTER(STATUS));
	if ((status & 0x80) != 0) {
		UARTSendFormat("状态寄存器读数错误：%d\r\n", status);
		goto on;
	}
	//  UARTSendFormat("Status: %d\r\n",status);
	if(status & RX_DR) {
		if (RX_P_NO_GET(status) > 5) {
			// UARTSendFormat("RX FIFO 为空或不可用\r\n");
			goto on;
		}
		spi_read_multi(c->spi, R_RX_PAYLOAD, data, *len);
		UARTSendFormat("读取 %d 字节，数据：%02X %02X %02X %02X，来源于管道号：%d\r\n",
		 *len, data[0], data[1], data[2], data[3], RX_P_NO_GET(status));
		// spi_write(spi, FLUSH_RX,0x00);
		// spi_write(spi, W_REGISTER(STATUS), status | RX_DR);
	} else {
		UARTSendFormat("Status: %02X\r\n", status);
	}
	// spi_write(spi, W_REGISTER(STATUS), status | RX_DR);
	// spi_write(spi, FLUSH_RX,0x00);
on:
	return 1;
}

void ci24r1_sel_spi(ci24r1_config_t *c) {
	spi_write(c->spi, SELSPI, 0);
}

void ci24r1_sel_irq(ci24r1_config_t *c) {
	spi_write(c->spi, SELIRQ, 0);
}

uint8_t ci24r1_irq(ci24r1_config_t *c) {
	// TODO 不应该使用 spi 的内部定义
	return c->spi->miso() == 0;
}

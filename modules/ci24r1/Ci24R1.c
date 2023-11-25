#include "Ci24R1.h"
#include "stc.h"

static const uint8_t addr[5] = {1,2,3,4,5};
static uint8 user_data[4] = {0x00, 0x00, 0x00, 0x00};


void Ci24R1_TX_Mode(void)
{
	SpiWrite(CE_OFF,0x00); //CE 拉低，芯片进入待机模式
	SpiWrite(W_REGISTER(SETUP_AW),      0x03); //设置接收地址宽度为 5 个字节;
	SpiWrites(W_REGISTER(TX_ADDR),      (uint8_t *)addr, 5);
	SpiWrites(W_REGISTER(RX_ADDR_P0),   (uint8_t*)addr,5);
	SpiWrite(W_REGISTER(EN_RXADDR),     ERX_P0 | REG0F_SEL_H_SET(2));
	SpiWrite(W_REGISTER(EN_AA),         ENAA_P0 | REG0F_SEL_L_SET(2));
	SpiWrite(W_REGISTER(OSC_CAP),       OSC_CAP_SET(0b1011));
	SpiWrite(W_REGISTER(FEATURE),       0);
	SpiWrite(W_REGISTER(RF_CH),         80);
	SpiWrite(W_REGISTER(RF_SETUP),      RF_DR_SET(2) | RF_PWR_SET(0));
	SpiWrite(W_REGISTER(SETUP_RETR),    ARD_SET(500) | ARC_SET(15));
	SpiWrite(W_REGISTER(CONFIG),        PWR_UP | EN_CRC | CRCO);
	// uint8_t config = SpiRead(R_REGISTER(CONFIG));
	// UARTSendFormat("配置寄存器读/写值一样? %d, %d\r\n", config, (PWR_UP | EN_CRC | CRCO));
	SpiWrite(CE_ON, 0x00);
}

uint8_t Ci24R1_TxPacket(void)
{
	if (++user_data[3]== 0) {
		if (++user_data[2] == 0) {
			if (++user_data[1] == 0) {
				++user_data[0];
			}
		}
	}
	SpiWrite(CE_OFF,0x00);
	SpiWrite(FLUSH_TX,0x00);
	SpiWrites(W_TX_PAYLOAD ,user_data,4); //写 TX FIFO
	SpiWrite(CE_ON,0x00); //CE 拉高，Ci24R1 开始发射 HAL_Delay(10); //等待数据发送完成
						  //
	while(1) {
		uint8_t status=SpiRead(R_REGISTER(STATUS));
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
			SpiWrite(W_REGISTER(STATUS), status | MAX_RT);
			break;
		} else if(status & TX_DS) {
			UARTSendFormat("数据发送完成: %02X %02X %02X %02X\r\n", user_data[0], user_data[1], user_data[2], user_data[3]);
			SpiWrite(W_REGISTER(STATUS), status);
			break;
		} else {
			UARTSendFormat("循环发送中: %d\r\n", status);
		}
	}
	
	// UARTSendString("退出发送\r\n");
	return 0xff;
}

void Ci24R1_RxMode(void) {
	SpiWrite(CE_OFF,0x00);
	SpiWrite(W_REGISTER(SETUP_AW),      0x03);
	SpiWrites(W_REGISTER(RX_ADDR_P0),   (uint8_t*)addr,5);
	SpiWrite(W_REGISTER(RX_PW_P0),      4);
	SpiWrite(W_REGISTER(EN_RXADDR),     ERX_P0 | REG0F_SEL_H_SET(2));
	SpiWrite(W_REGISTER(EN_AA),         ENAA_P0 | REG0F_SEL_L_SET(2));
	SpiWrite(W_REGISTER(OSC_CAP),       OSC_CAP_SET(0b1011));
	SpiWrite(W_REGISTER(FEATURE),       EN_DYN_ACK);
	SpiWrite(W_REGISTER(RF_CH),         80);
	SpiWrite(W_REGISTER(RF_SETUP),      RF_DR_SET(2) | RF_PWR_SET(0));
	SpiWrite(W_REGISTER(CONFIG),        (PWR_UP | EN_CRC | CRCO | PRIM_RX));
	// uint8_t config = SpiRead(R_REGISTER(CONFIG));
	// UARTSendFormat("配置寄存器读/写值一样? %d, %d\r\n", config, 0x0F);
	// sleep();
	// sleep();
	// sleep();
	SpiWrite(CE_ON,0x00);
}

uint8_t Ci24R1_RxPacket(void)
{
	SpiWrite(SELSPI, 0);
	uint8_t status = SpiRead(R_REGISTER(STATUS));
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
		uint8_t buf[4]={0};
		// uint8_tlen = SpiRead(R_RX_PL_WID);
		uint8_t len = 4;
		SpiReads(R_RX_PAYLOAD,buf, len);
		static int count = 0;
		UARTSendFormat("读取 %d 字节，数据：%02X %02X %02X %02X，来源于管道号：%d，序列号：%d\r\n",
		 len, buf[0], buf[1], buf[2], buf[3], RX_P_NO_GET(status), ++count);
		// SpiWrite(FLUSH_RX,0x00);
		// SpiWrite(W_REGISTER(STATUS), status | RX_DR);
	} else {
		UARTSendFormat("Status: %02X\r\n", status);
	}
	// SpiWrite(W_REGISTER(STATUS), status | RX_DR);
	// SpiWrite(FLUSH_RX,0x00);
on:
	return 1;
}

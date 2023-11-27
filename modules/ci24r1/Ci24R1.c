#include "Ci24R1.h"
#include <spi/spi.h>

static const uint8_t __code addr[5] = {1,2,3,4,5};

static void Ci24R1_TX_Mode(ci24r1_config_t *c);
static void Ci24R1_RX_Mode(ci24r1_config_t *c);

// TODO 删除
extern void UARTSendString(const int8_t *str);
extern void UARTSendFormat(const int8_t *format, ...);

// 判断发送是否处于空闲状态，如果 pending，则表示有数据等待发送。
static uint8_t ci24r1_pending_send(ci24r1_config_t *config);

uint8_t ci24r1_online(ci24r1_config_t *c) {
	ci24r1_sel_spi(c);
	uint8_t input = EN_DYN_ACK | EN_ACK_PAY | EN_DPL;
	spi_write(c->spi, W_REGISTER(FEATURE), input);
	uint8_t output = spi_read(c->spi, R_REGISTER(FEATURE));
	return output == input;
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

ci24r1_send_status_t ci24r1_send(ci24r1_config_t *c, const uint8_t *data, uint8_t len, uint8_t wait) {
	spi_config_t *spi = c->spi;

	while (wait && ci24r1_pending_send(c)) {
		// 忙等！
	}

	// spi_write(spi, CE_OFF,0x00);
	// spi_write(spi, FLUSH_TX,0x00);
	// 注意状态：写此寄存器必须不能在有数据正在发送的时候。
	// 所以：
	// 1. 本次发送前，等待 pending_send 返回 false
	// 2. 上一次调用 send 后，等待发送完
	spi_write_multi(spi, W_TX_PAYLOAD, data, len);

	if (!wait) {
		return CI24R1_SEND_STATUS_ASYNC;
	}

keep_waiting:
	uint8_t status=spi_read(spi, R_REGISTER(STATUS));
	if ((status & 0x80) != 0) {
		UARTSendFormat("状态寄存器读数错误：%d\r\n", status);
		return CI24R1_SEND_STATUS_FAIL;
	}

	// 因为总是在发送模式下写数据，所以不存在满的情况？
	// if (status & TX_FULL) {
	// 	UARTSendFormat("发送数据满了: status=%d\r\n", status);
	// 	return 0;
	// }
	
	if(status & MAX_RT) {
		UARTSendFormat("达到最大重发次数\r\n");
		// § 4.2.1 ACK 模式
		// MAX_RT 中断在清除之前不能进行下一步的数据发送
		// TODO 交给用户决定要不要自动清除
		spi_write(spi, W_REGISTER(STATUS), status | MAX_RT);
		spi_write(spi, FLUSH_TX, 0);
		return CI24R1_SEND_STATUS_MAX_RT;
	} else if(status & TX_DS) {
		spi_write(spi, W_REGISTER(STATUS), status | TX_DS);
		return CI24R1_SEND_STATUS_OK;
	}
	
	goto keep_waiting;
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
		return 1;
	}
	//  UARTSendFormat("Status: %d\r\n",status);
	if(!(status & RX_DR)) {
		UARTSendFormat("Status: %02X\r\n", status);
		return 0;
	}
	if (RX_P_NO_GET(status) > 5) {
		// 这里有个异步问题：清除 RX_DR 的时候，如果此时此刻接收到了数据怎么办？
		// 会导致收到的数据不会产生中断标记。所以本质上是不是不应该启用自动 ACK，
		// 而总是接收成功之后手动 ACK？
		spi_write(c->spi, W_REGISTER(STATUS), status | RX_DR);
		// UARTSendFormat("RX FIFO 为空或不可用\r\n");
		return 0;
	}
	spi_read_multi(c->spi, R_RX_PAYLOAD, data, *len);
	UARTSendFormat("读取 %d 字节，数据：%02X %02X %02X %02X，来源于管道号：%d\r\n",
	 *len, data[0], data[1], data[2], data[3], RX_P_NO_GET(status));
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

uint8_t ci24r1_pending_send(ci24r1_config_t *c) {
	uint8_t status = spi_read(c->spi, R_REGISTER(FIFO_STATUS));
	return !(status & TX_EMPTY);
}

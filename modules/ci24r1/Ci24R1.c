#include "Ci24R1.h"
#include <spi/spi.h>

static const uint8_t __code addr[4] = {0xFF, 0xFF, 0xFF, 0xFF};

// TODO 删除
extern void UARTSendString(const int8_t *str);
extern void UARTSendFormat(const int8_t *format, ...);

void ci24r1_init_config(ci24r1_config_t *c) {
	c->en_aa = 0x00;        // 默认全部关闭，因为模块是独立的，上电状态不能依靠MCU的复位状态
	c->en_rxaddr = 0x00;    // 默认全部关闭
	c->cap = 0b1011;
	c->rf_ch_id = 0;
	c->_config = EN_CRC | CRCO;
}

void ci24r1_init(ci24r1_config_t *c) {
	spi_write(c->spi, W_REGISTER(FEATURE),      0);
	
	uint8_t addr_len = 0;
	switch (c->addr_len) {
		case 3: addr_len = 0b01; break;
		case 4: addr_len = 0b10; break;
		case 5: addr_len = 0b11; break;
	}
	spi_write(c->spi, W_REGISTER(SETUP_AW),     addr_len);

	spi_write(c->spi, W_REGISTER(EN_RXADDR),    c->en_rxaddr | REG0F_SEL_H_SET(2));
	spi_write(c->spi, W_REGISTER(EN_AA),        c->en_aa | REG0F_SEL_L_SET(2));
	spi_write(c->spi, W_REGISTER(OSC_CAP),      OSC_CAP_SET(c->cap));

	spi_write(c->spi, W_REGISTER(RF_CH),        RF_CH_ID(c->rf_ch_id));

	spi_write(c->spi, W_REGISTER(RF_SETUP),     RF_DR_SET(2) | RF_PWR_SET(0));
	spi_write(c->spi, W_REGISTER(CONFIG),       c->_config);

	// 发送方的
	spi_write_multi(c->spi, W_REGISTER(TX_ADDR),       (uint8_t *)addr, 4);
	spi_write_multi(c->spi, W_REGISTER(RX_ADDR_P0),    (uint8_t*)addr,4);
	spi_write(c->spi, W_REGISTER(SETUP_RETR),             ARD_SET(4000) | ARC_SET(15));
	spi_write(c->spi, FLUSH_TX,                        0);
}

// 判断发送是否处于空闲状态，如果 pending，则表示有数据等待发送。
// static uint8_t ci24r1_pending_send(ci24r1_config_t *config);

uint8_t ci24r1_online(ci24r1_config_t *c) {
	ci24r1_sel_spi(c);
	uint8_t old = spi_read(c->spi, R_REGISTER(FEATURE));
	if (old != 0x00 || old != 0xFF) { return 1; }
	uint8_t input = EN_DYN_ACK | EN_ACK_PAY | EN_DPL;
	spi_write(c->spi, W_REGISTER(FEATURE), input);
	uint8_t output = spi_read(c->spi, R_REGISTER(FEATURE));
	spi_write(c->spi, W_REGISTER(FEATURE), old);
	return output == input;
}

void ci24r1_mode(ci24r1_config_t *c, ci24r1_mode_t mode) {
	switch (mode) {
		case CI24R1_MODE_TX:
			c->_config &= ~PRIM_RX;
			break;
		case CI24R1_MODE_RX:
			c->_config |= PRIM_RX;
			break;
	}
	spi_write(c->spi, W_REGISTER(CONFIG), c->_config);
}

ci24r1_send_status_t ci24r1_send(ci24r1_config_t *c, const uint8_t *data, uint8_t len, uint8_t wait) {
	spi_config_t *spi = c->spi;

	// 用非空来表示有数据待发送好像有 BUG
	// 会出现 发送完成 → 忙等 的问题，很是奇怪
	// while (wait && ci24r1_pending_send(c)) {
	// 	// 忙等！
	// 	UARTSendString("忙等！\r\n");
	// }
	// 

	// 如果不清空，在连续发送失败的情况下会满
	// 所以为了保证安全，尽可能发送前清空。
	// 因为 wait 情况下会等待发送成功或失败，不存在清空等待发送的数据。
	// 尝试过在 MAX_RT 的时候清空，效果不好
	spi_write(spi, FLUSH_TX, 0);

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
	if (status & TX_FULL) {
		return CI24R1_SEND_STATUS_FULL;
	} else if(status & MAX_RT) {
		UARTSendFormat("达到最大重发次数\r\n");
		// § 4.2.1 ACK 模式
		// MAX_RT 中断在清除之前不能进行下一步的数据发送
		// TODO 交给用户决定要不要自动清除
		spi_write(spi, W_REGISTER(STATUS), status | MAX_RT);
		// 此时队列是有数据的，不会自动清空，但是是发生失败的数据，留着没有用？
		// spi_write(spi, FLUSH_TX, 0);
		return CI24R1_SEND_STATUS_MAX_RT;
	} else if(status & TX_DS) {
		spi_write(spi, W_REGISTER(STATUS), status | TX_DS);
		return CI24R1_SEND_STATUS_OK;
	} else {
		UARTSendFormat("继续发送中，status=%d\r\n", status);
	}
	
	goto keep_waiting;
}

uint8_t ci24r1_recv(ci24r1_config_t *c, uint8_t *data, uint8_t *len) {
	uint8_t status = spi_read(c->spi, R_REGISTER(STATUS));
	if ((status & 0x80) != 0) {
		// UARTSendFormat("状态寄存器读数错误：%d\r\n", status);
		return 1;
	}
	//  UARTSendFormat("Status: %d\r\n",status);
	if(!(status & RX_DR)) {
		// UARTSendFormat("Status: %02X\r\n", status);
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
	UARTSendFormat("读取 %d 字节，数据：%02X %02X %02X %02X\r\n", *len, data[0], data[1], data[2], data[3]);
	return 0;
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

void ci24r1_shutdown(ci24r1_config_t *c, uint8_t yes) {
	if(yes) { c->_config &= ~ PWR_UP; }
	else    { c->_config |= PWR_UP; }
	spi_write(c->spi, W_REGISTER(CONFIG), c->_config);
}

void ci24r1_standby(ci24r1_config_t *c, uint8_t enter) {
	spi_write_byte(c->spi, enter ? CE_OFF : CE_ON);
}

void ci24r1_config_channel(ci24r1_config_t *c, uint8_t n, const ci24r1_channel_config_t* cc) {
	// 接收地址
	uint8_t addr = RX_ADDR_P0 + n;
	spi_write_multi(c->spi, W_REGISTER(addr), cc->addr, c->addr_len);

	// 数据长度（只支持定长）
	uint8_t pw_addr = RX_PW_P0 + n;
	spi_write(c->spi, W_REGISTER(pw_addr), cc->payload_width);
	
	// 自动应答模式
	uint8_t aa_mask = ENAA_P0 << n;
	if(cc->auto_ack)    c->en_aa |= aa_mask;
	else                c->en_aa &= ~aa_mask;
	spi_write(c->spi, W_REGISTER(EN_AA), c->en_aa);
	
	// 是否开启通道
	// 不在这里配置，有专门的 enable 函数
}

void ci24r1_enable_channel(ci24r1_config_t *c, uint8_t n, uint8_t enable) {
	uint8_t mask = ERX_P0 << n;
	if(enable)  { c->en_rxaddr |= mask; }
	else        { c->en_rxaddr &= ~mask; }
	spi_write(c->spi, W_REGISTER(EN_RXADDR), c->en_rxaddr);
}

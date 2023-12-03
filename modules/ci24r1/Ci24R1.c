#include <stdint.h>
#include <spi/spi.h>

#include "Ci24R1.h"
#include "reg.h"

#define ADDR_LEN 5

// TODO 删除
extern void UARTSendString(const int8_t *str);
extern void UARTSendFormat(const int8_t *format, ...);

uint8_t ci24r1_init(ci24r1_config_t *c, spi_config_t *spi) {
	c->spi = spi;

	// 默认全部关闭，因为模块是独立的，上电状态不能依靠MCU的复位状态
	// 默认选中电容配置。
	c->en_aa = 0x00 | REG0F_SEL_L_SET(2);
	c->en_rxaddr = 0x00 | REG0F_SEL_H_SET(2);

	c->config = EN_CRC | CRCO;

	spi_write(c->spi, SELSPI, 0);
	spi_write(c->spi, W_REGISTER(FEATURE),      0);
	spi_write(c->spi, W_REGISTER(SETUP_AW),     0b11);

	spi_write(c->spi, W_REGISTER(EN_RXADDR),    c->en_rxaddr);
	spi_write(c->spi, W_REGISTER(EN_AA),        c->en_aa);
	spi_write(c->spi, W_REGISTER(OSC_CAP),      OSC_CAP_SET(0b011));    // 官方推荐的电容值

	spi_write(c->spi, W_REGISTER(RF_CH),        RF_CH_ID(80)); // 默认随便一个，后续允许配置
	spi_write(c->spi, W_REGISTER(RF_SETUP),     RF_DR_SET(2) | RF_PWR_SET(0));

	spi_write(c->spi, W_REGISTER(CONFIG),       c->config);
	
	if(ci24r1_online(c)) {
		return 0;
	}
	return 1;
}

uint8_t ci24r1_online(ci24r1_config_t *c) {
	uint8_t old = spi_read(c->spi, R_REGISTER(FEATURE));
	if (old != 0x00 || old != 0xFF) { return 1; }
	uint8_t input = EN_DYN_ACK | EN_ACK_PAY | EN_DPL;
	spi_write(c->spi, W_REGISTER(FEATURE), input);
	uint8_t output = spi_read(c->spi, R_REGISTER(FEATURE));
	spi_write(c->spi, W_REGISTER(FEATURE), old);
	return output == input;
}

// 除正在发送数据是不能修改寄存器，其它时候均可以。
void ci24r1_state(ci24r1_config_t *c, uint8_t power_up, uint8_t standby, uint8_t tx) {
	if(power_up)    { c->config |=  PWR_UP;     }
	else            { c->config &= ~PWR_UP;     }
	
	if (tx)         { c->config &= ~PRIM_RX;    }
	else            { c->config |= PRIM_RX;     }

	spi_write(c->spi, W_REGISTER(CONFIG), c->config);

	spi_write_byte(c->spi, standby ? CE_OFF : CE_ON);
}

ci24r1_status_t ci24r1_send(ci24r1_config_t *c, const uint8_t *data, uint8_t len, uint8_t wait) {
	spi_config_t *spi = c->spi;

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
		return CI24R1_STATUS_ASYNC;
	}

keep_waiting:
	uint8_t status=spi_read(spi, R_REGISTER(STATUS));
	if ((status & 0x80) != 0) {
		return CI24R1_STATUS_OFFLINE;
	}

	// 因为总是在发送模式下写数据，所以不存在满的情况？
	if (status & TX_FULL) {
		return CI24R1_STATUS_FULL;
	} else if(status & MAX_RT) {
		// UARTSendFormat("达到最大重发次数\r\n");
		// § 4.2.1 ACK 模式
		// MAX_RT 中断在清除之前不能进行下一步的数据发送
		// TODO 交给用户决定要不要自动清除
		spi_write(spi, W_REGISTER(STATUS), status | MAX_RT);
		// 此时队列是有数据的，不会自动清空，但是是发生失败的数据，留着没有用？
		// spi_write(spi, FLUSH_TX, 0);
		return CI24R1_STATUS_MAX_RT;
	} else if(status & TX_DS) {
		spi_write(spi, W_REGISTER(STATUS), status | TX_DS);
		return CI24R1_STATUS_OK;
	} else {
		// UARTSendFormat("继续发送中，status=%d\r\n", status);
		// TODO 避免死循环？
		// 会在以下已知情况下出现死循环：
		// 1. 调用 send 时候并未 power_up = 1 standby = 0，也就是说未进入发送状态。
		//    本来库实现尽量为了精简，本身也没有几个函数，就未作此检测。
	}
	
	goto keep_waiting;
}

ci24r1_status_t ci24r1_recv(ci24r1_config_t *c, uint8_t *data, uint8_t len) {
	uint8_t status = spi_read(c->spi, R_REGISTER(STATUS));
	if ((status & 0x80) != 0) {
		return CI24R1_STATUS_OFFLINE;
	}
	if(!(status & RX_DR)) {
		return CI24R1_STATUS_NO_DATA;
	}
	if (RX_P_NO_GET(status) > 5) {
		// 这里有个异步问题：清除 RX_DR 的时候，如果此时此刻接收到了数据怎么办？
		// 会导致收到的数据不会产生中断标记。所以本质上是不是不应该启用自动 ACK，
		// 而总是接收成功之后手动 ACK？
		spi_write(c->spi, W_REGISTER(STATUS), status | RX_DR);
		return CI24R1_STATUS_NO_DATA;
	}
	spi_read_multi(c->spi, R_RX_PAYLOAD, data, len);
	return CI24R1_STATUS_OK;
}

void ci24r1_begin_irq(ci24r1_config_t *c) {
	spi_write(c->spi, SELIRQ, 0);
	// 手动拉高，如果有 IRQ，会被设备拉低
	// TODO 禁止使用内部实现。
	c->spi->mosi(1);
}

void ci24r1_end_irq(ci24r1_config_t *c) {
	spi_write(c->spi, SELSPI, 0);
}

uint8_t ci24r1_has_data(ci24r1_config_t *c, uint8_t irq) {
	if (irq) return c->spi->miso() == 0;

	uint8_t status = spi_read(c->spi, R_REGISTER(STATUS));
	if (status & RX_DR) {
		return 1;
	}

	return 0;
}

void ci24r1_config_tx(ci24r1_config_t *c, const uint8_t *addr) {
	spi_write_multi(c->spi, W_REGISTER(TX_ADDR),        addr, 5);
	// 自动应答要求接收通道 0 配置成和 发送 一样。 
	spi_write_multi(c->spi, W_REGISTER(RX_ADDR_P0),     addr, 5);
	// 自动重发延迟以及重发次数，暂时内部设置，不暴露出去以简化。
	spi_write(c->spi, W_REGISTER(SETUP_RETR),           ARD_SET(4000) | ARC_SET(15));
	spi_write(c->spi, FLUSH_TX,                         0);
}

void ci24r1_config_rx_channel(ci24r1_config_t *c, uint8_t n, uint8_t enable, const uint8_t *addr, uint8_t payload_width, uint8_t auto_ack) {
	// 接收地址
	uint8_t rx_addr = RX_ADDR_P0 + n;
	spi_write_multi(c->spi, W_REGISTER(rx_addr), addr, ADDR_LEN);

	// 数据长度（只支持定长）
	uint8_t pw_addr = RX_PW_P0 + n;
	spi_write(c->spi, W_REGISTER(pw_addr), payload_width);
	
	// 自动应答模式
	uint8_t aa_mask = ENAA_P0 << n;
	if(auto_ack)    c->en_aa |= aa_mask;
	else            c->en_aa &= ~aa_mask;
	spi_write(c->spi, W_REGISTER(EN_AA), c->en_aa);

	// 使能？
	uint8_t mask = ERX_P0 << n;
	if(enable)  { c->en_rxaddr |= mask; }
	else        { c->en_rxaddr &= ~mask; }
	spi_write(c->spi, W_REGISTER(EN_RXADDR), c->en_rxaddr);
}

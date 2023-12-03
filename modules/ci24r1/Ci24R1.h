#pragma once

#include <stdint.h>
#include <spi/spi.h>

// 为了避免外部动态申请内存，暴露内部定义。
typedef struct {
	spi_config_t *spi;
	
	// 如果寄存器读/写一致的话，这些其实没必须
	// 只是测试的时候发现写入和读取不一样，所以备份一下。
	uint8_t en_aa;
	uint8_t en_rxaddr;
	uint8_t config;
} ci24r1_config_t;

// 模块根据初始化。
// config 成员无需设置。
uint8_t ci24r1_init(ci24r1_config_t *config, spi_config_t *spi);

// 模式切换：上电？待机？发送？
void ci24r1_state(ci24r1_config_t *config, uint8_t power_up, uint8_t standby, uint8_t tx);

// 配置发送。
// 配置发送和配置接收不冲突，可以都配置，方便直接通过 state 函数改变状态。
// 地址强制为 5 个字节，内部不会缓存 addr 指针。
void ci24r1_config_tx(ci24r1_config_t *config, const uint8_t *addr);

// 配置接收通道。
// 内部会区别是读还是写的配置，所以应该先调用 mode 函数设置好正确的工作状态。
// addr 长度必须为初始化的地址长度大小。
// 可以调用多次以分别针对不同的通道进行配置。
// TODO 把 开/关 拆成独立函数，因为开关不影响配置，也无需传递地址之类的配置。
// 配置发送和配置接收不冲突，可以都配置，方便直接通过 state 函数改变状态。
// 负载长度必须和发送方统一好，并且只支持固定长度。实测动态长度有BUG（拿不到长度信息），代码删除了。
// 地址强制为 5 个字节，内部不会缓存 addr 指针。
void ci24r1_config_rx_channel(ci24r1_config_t *config, uint8_t n, uint8_t enable, const uint8_t *addr, uint8_t payload_width, uint8_t auto_ack);

// 判断设备是否在线。
// 用写、读、比较寄存器的方式实现，效率不高，但是准确。
uint8_t ci24r1_online(ci24r1_config_t *config);

typedef enum {
	CI24R1_STATUS_OK   = 0,     // 发送/接收成功
	CI24R1_STATUS_FAIL,         // 通用错误，未定义错误
	CI24R1_STATUS_ASYNC,        // wait=false 的情况下，异步发送中
	CI24R1_STATUS_MAX_RT,       // 发送失败，到达最大发送次数
	CI24R1_STATUS_FULL,         // 发送队列满了，不使用
	CI24R1_STATUS_OFFLINE,      // 设备离线
	CI24R1_STATUS_NO_DATA,      // 接收无数据
} ci24r1_status_t;

// 进入到 IRQ 模式等待数据，中断后，必须手动结束。
void ci24r1_begin_irq(ci24r1_config_t *config);

// 结束 IRQ。结束后才能操作（读写寄存器、读数据等）。
void ci24r1_end_irq(ci24r1_config_t *config);

// 判断是否有数据可以读。
// 返回：0 无数据，其它 有数据。
// 如果开启了 irq，则只判断 irq 线。否则查询 status 寄存器。
uint8_t ci24r1_has_data(ci24r1_config_t *c, uint8_t irq);

// 发送数据。
// 不建议在非发送模式下操作（比如待机状态下，虽然模块本身允许，但是接口实现会偏复杂，尚未支持）。
// data 的 len 必须和接收方统一。
// data 不会被缓存。
ci24r1_status_t ci24r1_send(ci24r1_config_t *config, const uint8_t *data, uint8_t len, uint8_t wait);

// 接收数据。
// 数据的长度必须是 config_rx_channel 时的 payload_width 大小，否则行为不可预期。 
ci24r1_status_t ci24r1_recv(ci24r1_config_t *config, uint8_t *data, uint8_t len);

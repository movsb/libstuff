#pragma once

#include <stdint.h>
#include <spi/spi.h>

/**
 * CI24R1 的驱动代码内部状态。
 * 
 * @note 为了避免外部动态申请内存，才暴露的内部定义。外部无需初始化即可调用 \ref ci24r1_init() 初始化并使用。
 */
typedef struct {
	spi_config_t *spi;
	
	// 如果寄存器读/写一致的话，这些其实没必须
	// 只是测试的时候发现写入和读取不一样，所以备份一下。
	uint8_t en_aa;
	uint8_t en_rxaddr;
	uint8_t config;
} ci24r1_config_t;

/**
 * @brief 模块初始化。
 * 
 * @param config    实例 \ref ci24r1_config_t 的指针，无需初始化。内部用于保存状态。
 * @param spi       三线 SPI 总线操作接口。
 */
uint8_t ci24r1_init(ci24r1_config_t *config, spi_config_t *spi);

/**
 * @brief 工作模式切换：上电？待机？发送？
 * 
 * 组合以下状态：U/u - 上电与否, T - 发送，R - 接收, E/e - 使能（CE_ON/OFF）
 *
 * 例如：UET 表示：上电，发送模式，使能，不用一次性配所有状态
 * 
 * @param states    字符串表示的状态列表，顺序切换。
 * 
 * @note 无效的状态被忽略。
 */
void ci24r1_state(ci24r1_config_t *config, const uint8_t *states);

/**
 * @brief 配置发送。
 *
 * 配置发送和配置接收不冲突，可以都配置，方便直接通过 state 函数改变状态。
 * 会自动清空队列。
 *
 * @param addr          通道的地址。必须是 \p ADDR_LEN 长度。
 */
void ci24r1_config_tx(ci24r1_config_t *config, const uint8_t *addr);

/**
 * @brief 配置接收通道。
 *
 * 可以调用多次以分别针对不同的通道进行配置。
 * 配置发送和配置接收不冲突，可以都配置，方便直接通过 state 函数改变状态。
 * 负载长度必须和发送方统一好，并且只支持固定长度。实测动态长度有BUG（拿不到长度信息），代码删除了。
 * 配置后会自动清空此队列的遗留数据。
 * 
 * @param n             第几个接收通道 [0,5] 共 6 个
 * @param enable        是否启用此通道
 * @param addr          通道的地址。必须是 \p ADDR_LEN 长度。
 * @param payload_width 接收数据的长度。 [1,32] 个字节。
 * @param auto_ack      是否开启自动应答。
 *
 * @todo 把 开/关 拆成独立函数，因为开关不影响配置，也无需传递地址之类的配置。
 */
void ci24r1_config_rx_channel(ci24r1_config_t *config, uint8_t n, uint8_t enable, const uint8_t *addr, uint8_t payload_width, uint8_t auto_ack);

/**
 * @brief 判断设备是否在线。
 *
 * 用写、读、比较寄存器的方式实现，效率不高，但是准确。
 * 
 * @return 布尔值。1 - 在线，其它 - 不在线
 */
uint8_t ci24r1_online(ci24r1_config_t *config);

/**
 * @brief 发送与接收状态返回值。
*/
typedef enum {
	CI24R1_STATUS_OK   = 0,     // 发送/接收成功
	CI24R1_STATUS_FAIL,         // 通用错误，未定义错误
	CI24R1_STATUS_ASYNC,        // wait=false 的情况下，异步发送中
	CI24R1_STATUS_MAX_RT,       // 发送失败，到达最大发送次数
	CI24R1_STATUS_FULL,         // 发送队列满了，不使用
	CI24R1_STATUS_OFFLINE,      // 设备离线
	CI24R1_STATUS_NO_DATA,      // 接收无数据
} ci24r1_status_t;

/**
 * @brief 进入 IRQ 模式等待数据。
 * 
 * 产生中断后，必须调用 \ref ci24r1_end_irq 手动结束。
 */
void ci24r1_begin_irq(ci24r1_config_t *config);

/**
 * @brief 结束 IRQ 模式。
 * 
 * 结束后才能操作（读写寄存器、读数据等）。
 */
void ci24r1_end_irq(ci24r1_config_t *config);

/**
 * @brief 判断是否有数据可以读。
 * 
 * @param irq   如果 irq = 1，则只判断 irq 状态。否则查询 status 寄存器。
 * 
 * @todo 内部保存 irq 的状态，并根据 irq 的状态自动决定怎么等数据。
 * 
 * @return 返回 0 表示无数据，其它表示有数据。
 */
uint8_t ci24r1_has_data(ci24r1_config_t *c, uint8_t irq);

/**
 * @brief 发送一个数据包。
 * 
 * @param data  待发送数据字节数组
 * @param len   待发送数据长度
 * @param wait  是否等待发送完成（失败或成功）
 * 
 * @note 待发送数据的长度 \p len 目前必须是 \ref ci24r1_config_rx_channel 时指定的 \p payload_width 大小，否则行为不可预期。
 * @note 函数内部实现不会持有 \p data ，即函数返回后可立即释放 \p data 。
 * @todo 内部保存 \p payload_width ，无需传递 \p len 参数。
 * 
 * @return 数据接收的状态。
 */
ci24r1_status_t ci24r1_send(ci24r1_config_t *config, const uint8_t *data, uint8_t len, uint8_t wait);

/**
 * @brief 接收一个数据包。
 * 
 * @param data  数据接收缓冲区
 * @param len   数组接收缓冲区的长度
 * 
 * @note 缓冲区 \p data 的长度必须是 \ref ci24r1_config_rx_channel 时指定的 \p payload_width 大小，否则行为不可预期。
 * @todo 内部保存 \p payload_width ，无需传递 \p len 参数。
 * 
 * @return 数据接收的状态。
 */
ci24r1_status_t ci24r1_recv(ci24r1_config_t *config, uint8_t *data, uint8_t len);

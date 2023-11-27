#pragma once

#include <stdint.h>

// TODO 复用 MOSI/MISO 的情况改变 IO 口的方向

typedef struct {
	void (*init_gpio)(void);
	void (*clock)(uint8_t value);
	void (*mosi)(uint8_t value);
	uint8_t (*miso)(void);
	void (*csn)(uint8_t value);
	void (*sleep)(void);
} spi_config_t;

void    spi_init(spi_config_t *config);
void    spi_write(spi_config_t *c, uint8_t cmd, uint8_t value);
void    spi_write_multi(spi_config_t *c, uint8_t cmd, const uint8_t *data, uint8_t len);
uint8_t spi_read(spi_config_t *c, uint8_t cmd);
void    spi_read_multi(spi_config_t *c, uint8_t cmd, uint8_t *data, uint8_t len);

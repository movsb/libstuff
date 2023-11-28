#include "spi.h"

static uint8_t spi_read_byte_no_csn(spi_config_t *c);
static void spi_write_byte_no_csn(spi_config_t *c, uint8_t value);

void spi_init(spi_config_t *c) {
	c->init_gpio();
	c->csn(0);
	c->csn(1);
	c->clock(0);
}

void spi_write_byte(spi_config_t *c, uint8_t value) {
	c->csn(0);
	spi_write_byte_no_csn(c, value);
	c->csn(1);
}

void spi_write_byte_no_csn(spi_config_t *c, uint8_t value) {
	c->clock(0);

	for (uint8_t i = 0; i < 8; i++) {
		c->clock(0);
		c->mosi(value & 0x80 ? 1 : 0);
		c->clock(1);
		value <<= 1;
		if (c->sleep) { c->sleep(); }
	}
	
	c->clock(0);
}

uint8_t spi_read_byte(spi_config_t *c) {
	c->csn(0);
	uint8_t value = spi_read_byte_no_csn(c);
	c->csn(1);
	return value;
}

uint8_t spi_read_byte_no_csn(spi_config_t *c) {
	uint8_t value = 0;

	c->clock(0);
	for (uint8_t i = 0; i< 8; i++) {
		value <<= 1;
		c->clock(1);
		value |= c->miso() ? 1 : 0;
		c->clock(0);
		if (c->sleep) { c->sleep(); }
	}

	return value;
}

void spi_write(spi_config_t *c, uint8_t cmd, uint8_t value) {
	c->csn(0);
	spi_write_byte_no_csn(c, cmd);
	spi_write_byte_no_csn(c, value);
	c->csn(1);
}

void spi_write_multi(spi_config_t *c, uint8_t cmd, const uint8_t *data, uint8_t len) {
	c->csn(0);
	spi_write_byte_no_csn(c, cmd);
	while (len > 0) {
		spi_write_byte_no_csn(c, *data);
		++data;
		--len;
	}
	c->csn(1);
}

uint8_t spi_read(spi_config_t *c, uint8_t cmd) {
	uint8_t value;
	c->csn(0);
	spi_write_byte_no_csn(c, cmd);
	value = spi_read_byte_no_csn(c);
	c->csn(1);
	return value;
}

void spi_read_multi(spi_config_t *c, uint8_t cmd, uint8_t *data, uint8_t len) {
	c->csn(0);
	spi_write_byte_no_csn(c, cmd);
	while(len > 0) {
		*data = spi_read_byte_no_csn(c);
		++data;
		--len;
	}
	c->csn(1);
}

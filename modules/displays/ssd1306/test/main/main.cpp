#include <cstdio>
#include <esp_log.h>
#include <driver/gpio.h>
#include <driver/i2c.h>
#include <ssd1306/ssd1306.hpp>

static constexpr auto SCL = GPIO_NUM_2;
static constexpr auto SDA = GPIO_NUM_3;


static const char *TAG = "i2c-simple-example";

#define I2C_MASTER_SCL_IO           SCL      /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO           SDA      /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM              I2C_NUM_0                          /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ          1000000                     /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS       1000

/**
 * @brief i2c master initialization
 */
static esp_err_t i2c_master_init(void)
{
	auto i2c_master_port = I2C_MASTER_NUM;

	i2c_config_t conf = {
		.mode = I2C_MODE_MASTER,
		.sda_io_num = I2C_MASTER_SDA_IO,
		.scl_io_num = I2C_MASTER_SCL_IO,
		.sda_pullup_en = GPIO_PULLUP_ENABLE,
		.scl_pullup_en = GPIO_PULLUP_ENABLE,
		.master = {
			.clk_speed = I2C_MASTER_FREQ_HZ,
		},
	};

	i2c_param_config(i2c_master_port, &conf);

	return i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

void i2c_write(uint8_t *buf, int len) {
	i2c_master_write_to_device(I2C_MASTER_NUM, 0x3C, buf, len, 1000);
	printf("wrote %u bytes\n", len);
}

	extern void s();

extern "C" void app_main() {
	ESP_LOGI("tag", "app_main");
	ESP_ERROR_CHECK(i2c_master_init());

	ESP_LOGI(TAG, "I2C initialized successfully");
	
	ebp::modules::displays::ssd1306::i2c_write = [](uint8_t *buf, int len) -> void {
		i2c_master_write_to_device(I2C_MASTER_NUM, 0x3C, buf, len, 0);
	};
	// s();
	
	auto ssd = ebp::modules::displays::ssd1306::SSD1306(128, 64);
	ssd.init();
	
	ESP_ERROR_CHECK(i2c_driver_delete(I2C_MASTER_NUM));
	ESP_LOGI(TAG, "I2C de-initialized successfully");
}

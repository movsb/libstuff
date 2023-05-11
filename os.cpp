#include <ebp/os.hpp>

#include <esp_vfs_dev.h>
#include <driver/uart.h>

namespace ebp {
namespace os {

static _File _StdIn  = _File(stdin);
static _File _StdOut = _File(stdout);
static _File _StdErr = _File(stderr);

_File &StdIn    = _StdIn;
_File &StdOut   = _StdOut;
_File &StdErr   = _StdErr;

esp_err_t bindStdioWithUART(uint8_t num)
{
	static bool bound = false;
	if (bound) { return ESP_OK; }
	
	// Initialize VFS & UART so we can use std::cout/cin
	setvbuf(stdin, nullptr, _IONBF, 0);

	// Install UART driver for interrupt-driven reads and writes
	if (auto err = uart_driver_install(0, SOC_UART_FIFO_LEN+1, 0, 0, NULL, 0); err != ESP_OK) {
		return err;
	}

	// Tell VFS to use UART driver
	esp_vfs_dev_uart_use_driver(num);
	esp_vfs_dev_uart_port_set_rx_line_endings(0, ESP_LINE_ENDINGS_CR);
	// Move the caret to the beginning of the next line on '\n'
	esp_vfs_dev_uart_port_set_tx_line_endings(0, ESP_LINE_ENDINGS_CRLF);
	
	return ESP_OK;
}

}
}

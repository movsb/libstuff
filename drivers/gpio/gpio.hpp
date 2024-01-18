#pragma once

#ifdef __STUFF_ESP32__
	#include "gpio_esp32.hpp"
#elif defined(__STUFF_CH32V003__)
	#include "gpio_ch32v003.hpp"
#endif

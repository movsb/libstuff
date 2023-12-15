#pragma once

#ifndef __STC_FOSC__
	#error 没定义时钟频率
	#define __STC_FOSC__ 11059200
#endif

#if defined(__STC_STC8G1K08A__)
	#define STC_GUID_ADDR   0x1FF9
	#define EEPROM_SIZE     (4 * 1024)
#elif defined(__STC_STC8G1K17A__)
	#define STC_GUID_ADDR   0x43F9
	#define EEPROM_SIZE     -1
#else // 以下的只是为了编辑器不报智能提示错误
	#error 需要指定芯片型号，例如：__STC_STC8G1K17A__
	#define STC_GUID_ADDR   0
	#define EEPROM_SIZE     0
#endif

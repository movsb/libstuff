#include <stddef.h>
#include <stdint.h>
#include "pri.h"
#include "stc.h"

// IAP & EEPROM
// STC 在 ROM 的最后 512 字节存储了 GUID 等数据，不要操作！
// 由于写 1 需要擦除整个 512 字节扇区，但是 RAM 又不太可能装得下，
// 所以操作逻辑： 把倒数第二个扇区用作临时备份扇区，改完指定的数据后写回来。
// 所以，倒数最后两个扇区用户不能使用！

#if EEPROM_SIZE == -1
	#if defined(__STC_EEPROM_SIZE__)
		#undef EEPROM_SIZE
		#define EEPROM_SIZE __STC_EEPROM_SIZE__
	#else
		#error 需要指定 EEPROM 的大小
	#endif
#elif EEPROM_SIZE > 0 && defined(__STC_EEPROM_SIZE__)
	#error EEPROM 大小已被型号确定，不能重复定义
#endif

#define EEPROM_RESERVED 1024
#if EEPROM_SIZE > 0 && EEPROM_SIZE < EEPROM_RESERVED
	#error EEPROM 大小不合适
#endif

#define SECTOR_BITS     9
#define SECTOR_SIZE     ((uint16_t)(1) << SECTOR_BITS) // 扇区大小（不能改，搞这么大搞屁啊😧）

// TODO 没四舍五入
inline static uint8_t _IAP_TPS(void) { return (uint8_t)((uint32_t)(__STC_FOSC__) / 1e6); }
inline static uint8_t _IAP_SectorOf(uint16_t addr) { return addr >> SECTOR_BITS; }
#define _IAP_Trig() do { IAP_TRIG = 0x5A; IAP_TRIG = 0xA5; NOP(); NOP(); } while(0)

uint8_t _IAP_Read(uint16_t addr, uint8_t *buf, uint8_t len);

uint8_t EEPROM_Read(uint16_t addr, uint8_t *buf, uint8_t len) {
	// 超出边界了
	if (EEPROM_SIZE == 0 || addr + len > EEPROM_SIZE - EEPROM_RESERVED) {
		return 0;
	}

	return _IAP_Read(addr, buf, len);
}

uint8_t _IAP_Read(uint16_t addr, uint8_t *buf, uint8_t len) {
	IAP_CONTR   |= IAPEN;
	IAP_TPS     = _IAP_TPS();

	for(uint8_t i = 0; i < len; i++) {
		IAP_ADDRL   = (uint8_t)(addr+i);
		IAP_ADDRH   = (uint8_t)((addr+i) >> 8);
		IAP_CMD     = IAP_CMD_READ;
		_IAP_Trig();
		
		if (IAP_CONTR & CMD_FAIL) {
			IAP_CONTR ^= IAPEN | CMD_FAIL;
			return 0;
		}

		buf[i] = IAP_DATA;
	}
	
	IAP_CONTR ^= IAPEN;
	return 1;
}

// 写入一个字节，一定是可写的
static uint8_t _IAP_WriteByte(uint16_t addr, uint8_t value) {
	IAP_CONTR   |= IAPEN;
	IAP_TPS     = _IAP_TPS();

	IAP_ADDRL   = (uint8_t)addr;
	IAP_ADDRH   = (uint8_t)(addr >> 8);
	IAP_DATA    = value;
	IAP_CMD     = IAP_CMD_WRITE;
	_IAP_Trig();
		
	if (IAP_CONTR & CMD_FAIL) {
		IAP_CONTR ^= IAPEN | CMD_FAIL;
		return 0;
	}

	IAP_CONTR ^= IAPEN;
	return 1;
}

// 擦除某个扇区
static uint8_t _IAP_EraseSector(uint8_t sector) {
	IAP_CONTR   |= IAPEN;
	IAP_TPS     = _IAP_TPS();
	
	uint16_t addr = (uint16_t)(sector * SECTOR_SIZE);
	
	IAP_ADDRL   = (uint8_t)addr;
	IAP_ADDRH   = (uint8_t)(addr >> 8);
	IAP_CMD     = IAP_CMD_ERASE;
	_IAP_Trig();
		
	if (IAP_CONTR & CMD_FAIL) {
		IAP_CONTR ^= IAPEN | CMD_FAIL;
		return 0;
	}

	IAP_CONTR ^= IAPEN;
	return 1;
}

// 先读，如果数据不需要擦除，则不擦
// 一般认为目标字节不为 0xFF 时，必须先擦除。
// 有极端情况：比如把 0x11 改成 0x10，其实不用擦，未作此特殊处理
static uint8_t _IAP_NeedToErase(uint16_t addr, const uint8_t *buf, uint8_t len, uint8_t *need) {
	*need = 0;
	uint8_t c;
	for (uint8_t i = 0; i < len; i++) {
		uint8_t r = _IAP_Read(addr, &c, 1);
		// 读失败
		if (!r) { return 0; }
		// 没变化 || 没写过，不擦
		if (c == buf[i] || c == 0xFF) { continue; }
		// 需要擦
		*need = 1;
		break;
	}
	return 1;
}

// 本来之前是用回调方式改每个字节的，但是发现编译器BUG，用户数据传过去后变成空了。。。
static uint8_t _IAP_CopySector(uint8_t from, uint8_t to, uint16_t addr, const uint8_t *buf, uint8_t len) {
	if (from == to) { return 1; }
	
	uint16_t fromAddr = (uint16_t)(from) * SECTOR_SIZE;
	uint16_t toAddr = (uint16_t)(to) * SECTOR_SIZE;
	uint16_t addrOffset = addr & (SECTOR_SIZE - 1);
	
	for (uint16_t i = 0; i < SECTOR_SIZE; i++) {
		uint8_t r;
		if (!_IAP_Read(fromAddr+i, &r, 1)) {
			return 0;
		}

		if (buf != NULL && i >= addrOffset && i < addrOffset + len) {
			r = buf[i];
		}

		if (!_IAP_WriteByte(toAddr+i, r)) {
			return 0;
		}
	}

	return 1;
}

// EEPROM 的写操作只能将字节中的 1 写为 0，当需要将字节中的 0 写为 1，则必须执行扇区擦除操作。
// EEPROM 的读/写操作是以 1 字节为单位进行，而 EEPROM 擦除操作是以 1 扇区（512 字节）为单位进行，
// 在执行擦除操作时，如果目标扇区中有需要保留的数据，则必须预先将这些数据读取到 RAM 中暂存，
// 待擦除完成后再将保存的数据和需要更新的数据一起再写回 EEPROM/DATA-FLASH。
uint8_t EEPROM_Write(uint16_t addr, const uint8_t *buf, uint8_t len) {
	// 超出边界了
	if (EEPROM_SIZE == 0 || addr + len > EEPROM_SIZE - EEPROM_RESERVED) {
		return 0;
	}

	// 不需要擦的话，直接写指定地址就行
	uint8_t need_to_erase;
	if (!_IAP_NeedToErase(addr, buf, len, &need_to_erase)) {
		return 0;
	}
	// 那就不擦
	if (!need_to_erase) {
		for(uint8_t i = 0; i < len; i++) {
			if (!_IAP_WriteByte(addr + i, buf[i])) {
				return 0;
			}
		}
		return 1;
	}
	
	// 需要擦除的过程：
	// 0. 擦除临时扇区
	// 1. 把当前扇区拷贝到临时扇区
	// 2. 擦除当前扇区
	// 3. 读取临时扇区
	//    修改指定地址数据
	//    写入
	// 真复杂。。。

	uint16_t tmpSectorAddr = EEPROM_SIZE - EEPROM_RESERVED;

	// 0. 我擦！怎么失败了。🤣
	if (!_IAP_EraseSector(_IAP_SectorOf(tmpSectorAddr))) {
		return 0;
	}
	// 1
	if (!_IAP_CopySector(_IAP_SectorOf(addr), _IAP_SectorOf(tmpSectorAddr), 0, NULL, 0)) {
		return 0;
	}
	// 2
	if (!_IAP_EraseSector(_IAP_SectorOf(addr))) {
		return 0;
	}
	// 3
	if (!_IAP_CopySector(_IAP_SectorOf(tmpSectorAddr), _IAP_SectorOf(addr), addr, buf, len)) {
		return 0;
	}
	
	return 1;
}

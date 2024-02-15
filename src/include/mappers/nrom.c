#include "../mapper.h"
#include <stdint.h>

uint8_t cpu_read(Mapper *mapper, uint16_t address);
uint8_t cpu_write(Mapper *mapper, uint16_t address, uint8_t data);
uint8_t ppu_read(Mapper *mapper, uint16_t address);
uint8_t ppu_write(Mapper *mapper, uint16_t address, uint8_t data);

void load_NROM(Mapper *mapper) {
	mapper->cpu_read = cpu_read;
	mapper->cpu_write = cpu_write;
	mapper->ppu_read = ppu_read;
	mapper->ppu_write = ppu_write;
}

uint8_t cpu_read(Mapper *mapper, uint16_t address) {
	uint8_t data = 0x00;
	if (address >= 0x8000 && address <= 0xFFFF) {
		// Data mirrored according to bank count
		address &= (mapper->PRG_ROM_banks > 1) ? 0x7FFF : 0x3FFF;
		data = mapper->PRG_ROM_p[address];
	}
	return data;
}

uint8_t cpu_write(Mapper *mapper, uint16_t address, uint8_t data) {
	//if (address >= 0x8000 && address <= 0xFFFF) {
	//	address &= (mapper->PRG_ROM_banks > 1) ? 0x7FFF : 0x3FFF;
	//	mapper->PRG_ROM_p[address] = data;
	//}
	//--- For future ---
	return 0;
}

uint8_t ppu_read(Mapper *mapper, uint16_t address) {
	uint8_t data = 0x00;
	if (address >= 0x0000 && address <= 0x1FFF)
		data = mapper->CHR_ROM_p[address];
	return data;
}

uint8_t ppu_write(Mapper *mapper, uint16_t address, uint8_t data) {
	//--- For future ---
	return 0;
}

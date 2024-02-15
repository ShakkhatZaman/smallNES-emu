#include "ppu.h"

Byte cpu_to_ppu_read(PPU *p_ppu, Word address) {
	address &= 0x0007;
	switch (address) {
		case 0x0:
			break;
		case 0x1:
			break;
		case 0x2:
			break;
		case 0x3:
			break;
		case 0x4:
			break;
		case 0x5:
			break;
		case 0x6:
			break;
		case 0x7:
			break;
	}
	return p_ppu->PPU_registers[address];
}

Byte cpu_to_ppu_write(PPU *p_ppu, Word address, Byte data) {
	address &= 0x0007;
	switch (address) {
		case 0x0:
			break;
		case 0x1:
			break;
		case 0x2:
			break;
		case 0x3:
			break;
		case 0x4:
			break;
		case 0x5:
			break;
		case 0x6:
			break;
		case 0x7:
			break;
	}
	return 0; // Placeholder
}

Byte ppu_read_byte(PPU *p_ppu, Word address) {
	Byte data = 0x00;
	address &= 0x3FFF;
	if (address >= 0x0000 && address <= 0x1FFF)
		p_ppu->p_Bus->mapper->ppu_read(p_ppu->p_Bus->mapper, address);

	return data;
}

Byte ppu_write_byte(PPU *p_ppu, Word address, Byte data) {
	address &= 0x3FFF;
	if (address >= 0x0000 && address <= 0x1FFF)
		p_ppu->p_Bus->mapper->ppu_write(p_ppu->p_Bus->mapper, address, data);

	return 0;
}


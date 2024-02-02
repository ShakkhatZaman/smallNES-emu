#include "ppu.h"

Byte cpu_to_ppu_read(PPU *p_ppu, Word address){
	address &= 0x0007;
	return p_ppu->PPU_registers[address];
}

Byte cpu_to_ppu_write(PPU *p_ppu, Word address, Byte data){
	address &= 0x0007;
	return 0; // Placeholder
}

Byte ppu_read_byte(PPU *p_ppu, Word address); // define later

Byte ppu_write_byte(PPU *p_ppu, Word address, Byte data);


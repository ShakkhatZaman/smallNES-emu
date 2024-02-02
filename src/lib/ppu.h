#ifndef PPU_H
#define PPU_H

#include <stdint.h>
#include "mapper.h"

typedef uint8_t Byte;
typedef uint16_t Word;

typedef struct {
	Byte Pattern[2][4096];	// 8KB for pattern tables ->	$0000 - $1FFF //programed by the cartridge
	Byte Nametable[4][1024];// 4KB for nametables ->		$2000 - $2FFF
	Byte Paletes[32];		// 32B for palettes ->			$3F00 - $3FFF
	Mapper *mapper;
} PPU_Bus;

typedef struct {
	Byte PPU_registers[8];
	PPU_Bus *p_Bus;
} PPU;

Byte ppu_read_byte(PPU *p_ppu, Word address);

Byte ppu_write_byte(PPU *p_ppu, Word address, Byte data);

Byte cpu_to_ppu_read(PPU *p_ppu, Word address);

Byte cpu_to_ppu_write(PPU *p_ppu, Word address, Byte data);

#endif 

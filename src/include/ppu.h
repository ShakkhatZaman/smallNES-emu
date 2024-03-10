#ifndef PPU_H
#define PPU_H

#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "mapper.h"

typedef uint8_t Byte;
typedef uint16_t Word;

#define DOTS 341
#define SCANLINES 261

typedef struct {
	Byte Nametable[4][1024];// 4KB for nametables ->		$2000 - $2FFF
	Byte Paletes[32];		// 32B for palettes ->			$3F00 - $3FFF
	Mapper *mapper;
} PPU_Bus;

typedef struct {
	Byte PPU_registers[8];
	PPU_Bus *p_Bus;
    int dots;
    int scanlines;
    uint32_t screen_buffer[DOTS * SCANLINES];
    SDL_Texture *ppu_draw_texture;
    bool frame_complete;
} PPU;

void reset_ppu(PPU *ppu, PPU_Bus *ppu_bus);

int init_ppu(PPU *ppu, SDL_Renderer *renderer);

void ppu_clock(PPU *ppu);

Byte ppu_read_byte(PPU *p_ppu, Word address);

Byte ppu_write_byte(PPU *p_ppu, Word address, Byte data);

Byte cpu_to_ppu_read(PPU *p_ppu, Word address);

Byte cpu_to_ppu_write(PPU *p_ppu, Word address, Byte data);

#endif //PPU_H

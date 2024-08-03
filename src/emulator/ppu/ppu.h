#ifndef PPU_H
#define PPU_H

#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#include "ppu_registers.h"
#include "../cartridge/mapper.h"
#include "../../types.h"

#define NES_WIDTH 256
#define NES_HEIGHT 240

#define COLORS_PER_PALETTE 4

#define DOTS 341
#define SCANLINES 261


typedef struct {
    Byte Nametable[4][1024];// 4KB for 4 nametables ->		$2000 - $2FFF
    Byte Palettes[32];		// 32B for palettes ->			$3F00 - $3FFF
    Mapper *mapper;
} PPU_Bus;

typedef struct {
    Byte PPU_registers[8];
    // Registers
    PPUCTRL_reg PPUCTRL;
    PPUMASK_reg PPUMASK;
    PPUSTATUS_reg PPUSTATUS;
    VRAM_ADDR_reg current_address;
    VRAM_ADDR_reg temp_address;
    Byte fine_x;
    Byte PPUDATA;
    Byte write_latch;
    // Bus
    PPU_Bus *p_Bus;
    // Helper members
    Byte VRAM_increment;
    int dots;
    int scanlines;
    uint32_t screen_buffer[NES_WIDTH * NES_HEIGHT];
    SDL_Texture *ppu_draw_texture;
    bool frame_complete;
    bool create_nmi;
} PPU;

typedef struct {
    Byte LS_Byte;
    Byte MS_Byte;
} Pattern_row;

void reset_ppu(PPU *ppu, PPU_Bus *ppu_bus);

int init_ppu(PPU *ppu, SDL_Renderer *renderer);

void ppu_clock(PPU *ppu);

Byte ppu_read_byte(PPU *p_ppu, Word address);

Byte ppu_write_byte(PPU *p_ppu, Word address, Byte data);

Byte cpu_to_ppu_read(PPU *p_ppu, Word address);

Byte cpu_to_ppu_write(PPU *p_ppu, Word address, Byte data);

#endif //PPU_H

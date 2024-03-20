#ifndef PPU_H
#define PPU_H

#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#include "mapper.h"
#include "ppu_registers.h"
#include "types.h"

#define NES_WIDTH 256
#define NES_HEIGHT 240

#define COLORS_PER_PALETTE 4

#define DOTS 341
#define SCANLINES 261


typedef struct {
    Byte Nametable[4][1024];// 4KB for nametables ->		$2000 - $2FFF
    Byte Palettes[32];		// 32B for palettes ->			$3F00 - $3FFF
    Mapper *mapper;
} PPU_Bus;

typedef struct {
    Byte PPU_registers[8];
    // Registers
    PPUCTRL_reg PPUCTRL;
    PPUMASK_reg PPUMASK;
    PPUSTATUS_reg PPUSTATUS;
    Word PPUADDR;
    Byte PPUDATA;
    // Bus
    PPU_Bus *p_Bus;
    // Helper members
    Byte write_latch;
    Byte VRAM_increment;
    int dots;
    int scanlines;
    //DEBUG
    // uint32_t screen_buffer[DOTS * SCANLINES];
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

// DEBUG
Pattern_row get_pattern_row(PPU *ppu, Byte table_index, Byte plane_num, Byte plane_y);
uint32_t get_pixel_color(PPU *ppu, Byte palette_num, Byte pixel);
void draw_pixel_row(PPU *ppu, Pattern_row pattern_row, uint32_t *buffer, Byte palette_num, int row_x, int y);

#endif //PPU_H

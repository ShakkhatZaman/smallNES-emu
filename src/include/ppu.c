#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "ppu.h"
#include "ppu_registers.h"

static void ppu_draw(PPU *ppu);
static Pattern_row get_pattern_row(PPU *ppu, Byte table_index, Byte tile_num, Byte tile_y);
static uint32_t get_pixel_color(PPU *ppu, Byte palette_num, Byte pixel);

void ppu_clock(PPU *ppu) {
    if (ppu->dots >= DOTS) {
        ppu->scanlines++;
        if (ppu->scanlines >= SCANLINES) {
            ppu->scanlines = -1;
            ppu->frame_complete = true;
            SDL_UpdateTexture(ppu->ppu_draw_texture, NULL, ppu->screen_buffer, DOTS * 4);
        }
        if (ppu->scanlines > 240) ppu->PPUSTATUS.Verticle_blank = 1;
        ppu->dots = 0;
    }
    ppu_draw(ppu);
    ppu->dots++;
}

uint32_t NES_Palette[64] = {
    0x525201, 0x52511A, 0x0F0F23, 0x656306, 0x033640, 0x4B2604, 0x093F32, 0x040013, 0x201F0B, 0x00002A, 0x2F0000, 0x000A2E, 0x260000, 0x2D0000, 0x000000, 0x000000,
    0xA0A01E, 0xA09D4A, 0x373858, 0xBCB828, 0x217584, 0x945C23, 0x2E826F, 0x24003F, 0x525131, 0x000063, 0x6B1A0E, 0x052E69, 0x5C1000, 0x680000, 0x000000, 0x000000,
    0xFFFE69, 0xFFFC9E, 0x8789AE, 0xFFFF76, 0x6DCEE0, 0xF1B270, 0x7CDEC8, 0x703E91, 0xA7A681, 0x2528BA, 0xC46354, 0x467DC1, 0xB3563C, 0xC03C3C, 0x000000, 0x000000,
    0xFFFEBE, 0xFFFDD6, 0xCCCCDD, 0xFFFFC4, 0xC0EAF2, 0xF9DFC1, 0xC7F1E8, 0xC2AAD0, 0xDAD9C9, 0x9D9EE2, 0xE6BCB4, 0xAEC7E5, 0xDFB5A9, 0xE4A9A9, 0x000000, 0x000000
};

static void ppu_draw(PPU *ppu) {
    if (-1 < ppu->dots && ppu->dots < DOTS && -1 < ppu->scanlines && ppu->scanlines < SCANLINES) {
        ppu->screen_buffer[ppu->scanlines * DOTS + ppu->dots] = (rand() % 2) ? 0x00FFFFFF : 0x000000FF;
    }   // Noise for now
    // puts("frame");
    // Added for debugging

    // TODO fill the function
}

void reset_ppu(PPU *ppu, PPU_Bus *ppu_bus) {
    *ppu = (PPU) {
        .PPUCTRL =  0, .PPUMASK =  0,   // ALL registers initialized with 0
        .PPUSTATUS =  0, .PPUADDR = 0,
        .PPUDATA = 0,
        .p_Bus = ppu_bus,
        .write_latch = 0,               // Write latch to 0
        .VRAM_increment = 1,            // Default VRAM address increment to 1
        .dots = 0, .scanlines = 0,      // Number of dots and scanlines to 0
        .frame_complete = false         // Frame complete to false
    };
    memset(ppu->screen_buffer, 0, DOTS * SCANLINES * sizeof(uint32_t));
    memset(ppu->p_Bus->Palettes, 0, 0x1F);
    for (int i = 0; i < 4; i++)
        memset(ppu->p_Bus->Nametable[i], 0, 1024);
}

int init_ppu(PPU *ppu, SDL_Renderer *renderer) {
    int pixel_format = SDL_PIXELFORMAT_RGB888;
    SDL_Texture *texture = SDL_CreateTexture(renderer,
                                             pixel_format,
                                             SDL_TEXTUREACCESS_STREAMING,
                                             DOTS, SCANLINES);

    if (texture == NULL) return -1;
    ppu->ppu_draw_texture = texture;
    SDL_SetRenderTarget(renderer, ppu->ppu_draw_texture);
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderTarget(renderer, NULL);
    return 0;
}

Byte cpu_to_ppu_read(PPU *p_ppu, Word address) {
	address &= 0x0007;
    Byte data = 0;
	switch (address) {
		case 0x0: //PPUCTRL *** WRITE only ***
			break;

		case 0x1: //PPUMASK *** WRITE only ***
			break;

		case 0x2: //PPUSTATUS *** READ only ***
            p_ppu->PPUSTATUS.PPU_open_bus = p_ppu->PPUDATA & 0x1F;
            data = p_ppu->PPUSTATUS._;
            p_ppu->PPUSTATUS.Verticle_blank = 0;
            p_ppu->write_latch = 0;
			break;

		case 0x3: //OAMADDR *** WRITE only ***
			break;

		case 0x4: //OAMDATA *** READ / WRITE ***
			break;

		case 0x5: //PPUSCROLL *** WRITE only ***
			break;

		case 0x6: //PPUADDR *** WRITE only ***
			break;

		case 0x7: //PPUDATA *** READ / WRITE ***
            data = p_ppu->PPUDATA;
            p_ppu->PPUDATA = ppu_read_byte(p_ppu, p_ppu->PPUADDR);
            if (p_ppu->PPUADDR >= 0x3F00) data = p_ppu->PPUDATA;
            if ((p_ppu->PPUADDR + p_ppu->VRAM_increment) <= 0x3FFF) p_ppu->PPUADDR += p_ppu->VRAM_increment;
			break;
	}
	return data;
}

Byte cpu_to_ppu_write(PPU *p_ppu, Word address, Byte data) {
	address &= 0x0007;
	switch (address) {
		case 0x0: //PPUCTRL *** WRITE only ***
            p_ppu->PPUCTRL = (PPUCTRL_reg) data;
            p_ppu->VRAM_increment = (p_ppu->PPUCTRL.VRAM_address_inc) ? 32 : 1;
			break;

		case 0x1: //PPUMASK *** WRITE only ***
            p_ppu->PPUMASK = (PPUMASK_reg) data;
			break;

		case 0x2: //PPUSTATUS *** READ only ***
			break;

		case 0x3: //OAMADDR *** WRITE only ***
			break;

		case 0x4: //OAMDATA *** READ / WRITE ***
			break;

		case 0x5: //PPUSCROLL *** WRITE only ***
			break;

		case 0x6: //PPUADDR *** WRITE only ***
            if (!p_ppu->write_latch) {
                p_ppu->PPUADDR |= ((Word) data) << 8;
                p_ppu->write_latch = 1;
            }
            else {
                p_ppu->PPUADDR |= (Word) data;
                p_ppu->write_latch = 0;
            }
			break;

		case 0x7: //PPUDATA *** READ / WRITE ***
            p_ppu->PPUDATA = data;
            ppu_write_byte(p_ppu, p_ppu->PPUADDR, data);
            if ((p_ppu->PPUADDR + p_ppu->VRAM_increment) <= 0x3FFF) p_ppu->PPUADDR += p_ppu->VRAM_increment;
			break;
	}
	return 0; // Placeholder
}

Byte ppu_read_byte(PPU *p_ppu, Word address) {
	Byte data = 0x00;
	address &= 0x3FFF;
    // Inside CHR_ROM or pattern tables
    if (address <= 0x1FFF)
		p_ppu->p_Bus->mapper->ppu_read(p_ppu->p_Bus->mapper, address);
    // Inside Nametable memory
    else if (0x2000 >= address && address <= 0x2FFF) {
        Byte table_index = (address >> 10) & 0x3;
        address &= 0x3FF;
        data = p_ppu->p_Bus->Nametable[table_index][address];
    } // Inside Palette memory
    else if (0x3F00 >= address && address <= 0x3FFF) {
        address &= 0x1F;
        address = (address == 0x10) ? 0x00 : address;
        address = (address == 0x14) ? 0x04 : address;
        address = (address == 0x18) ? 0x08 : address;
        address = (address == 0x1C) ? 0x0C : address;
        data = p_ppu->p_Bus->Palettes[address];
    }

	return data;
}

Byte ppu_write_byte(PPU *p_ppu, Word address, Byte data) {
	address &= 0x3FFF;
    // Inside CHR_ROM or pattern tables
    if (address <= 0x1FFF)
        p_ppu->p_Bus->mapper->ppu_write(p_ppu->p_Bus->mapper, address, data);
    // Inside Nametable memory
    else if (0x2000 >= address && address <= 0x2FFF) {
        Byte table_index = (address >> 10) & 0x3;
        address &= 0x3FF;
        p_ppu->p_Bus->Nametable[table_index][address] = data;
    } // Inside Palette memory
    else if (0x3F00 >= address && address <= 0x3FFF) {
        address &= 0x1F;
        address = (address == 0x10) ? 0x00 : address;
        address = (address == 0x14) ? 0x04 : address;
        address = (address == 0x18) ? 0x08 : address;
        address = (address == 0x1C) ? 0x0C : address;
        p_ppu->p_Bus->Palettes[address] = data;
    }

    return 0;
}

static Pattern_row get_pattern_row(PPU *ppu, Byte table_index, Byte tile_num, Byte tile_y) {
    Word address = (table_index) ? 0x1000 : 0x0000;
    address |= ((Word) tile_num) << 4;
    address |= (tile_y < 8) ? tile_y : 0;
    Pattern_row row;
    row.LS_Byte = ppu_read_byte(ppu, address);
    address |= 0x8;
    row.MS_Byte = ppu_read_byte(ppu, address);
    return row;
}

static uint32_t get_pixel_color(PPU *ppu, Byte palette_num, Byte pixel) {
    Byte pixel_index = ppu_read_byte(ppu, (palette_num * COLORS_PER_PALETTE) + pixel);
    return NES_Palette[pixel_index & 0x3F];
}


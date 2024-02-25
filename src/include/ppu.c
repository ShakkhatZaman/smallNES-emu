#include <SDL2/SDL_render.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "ppu.h"

static void ppu_draw(PPU *ppu);

void ppu_clock(PPU *ppu) {
    if (ppu->dots >= DOTS) {
        ppu->scanlines++;
        if (ppu->scanlines >= SCANLINES) {
            ppu->scanlines = -1;
            ppu->frame_complete = true;
            SDL_UpdateTexture(ppu->ppu_draw_texture, NULL, ppu->screen_buffer, DOTS * 4);
        }
        ppu->dots = 0;
    }
    ppu_draw(ppu);
    ppu->dots++;
}

static void ppu_draw(PPU *ppu) {
    /* if (-1 < ppu->dots && ppu->dots < DOTS && -1 < ppu->scanlines && ppu->scanlines < SCANLINES) {
        ppu->screen_buffer[ppu->scanlines * DOTS + ppu->dots] = (rand() % 2) ? 0xFFFFFFFF : 0x000000FF;
    }   // Noise for now */
    // Added for debugging

    // TODO fill the function
}

void reset_ppu(PPU *ppu, PPU_Bus *ppu_bus) {
    ppu->p_Bus = ppu_bus;
    ppu->scanlines = -1;
    ppu->dots = 0;
}

int init_ppu(PPU *ppu, SDL_Renderer *renderer) {
    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, DOTS, SCANLINES);
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
	if (address <= 0x1FFF)
		p_ppu->p_Bus->mapper->ppu_read(p_ppu->p_Bus->mapper, address);

	return data;
}

Byte ppu_write_byte(PPU *p_ppu, Word address, Byte data) {
	address &= 0x3FFF;
	if (address <= 0x1FFF)
		p_ppu->p_Bus->mapper->ppu_write(p_ppu->p_Bus->mapper, address, data);

	return 0;
}


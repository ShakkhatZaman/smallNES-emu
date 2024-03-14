#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdio.h>

#include "debug.h"
#include "ppu.h"

#define PALETTE_PIXEL_WIDTH 4

SDL_Texture *debug_texture;
uint32_t debug_screen_buffer[256 * 240];
int debug_pixel_format = SDL_PIXELFORMAT_RGB888;

SDL_Rect debug_rect = {
    .w = 512, .h = 480,
    .x = 512, .y = 0
};

SDL_Rect ppu_screen_rect = {
    .w = 512, .h = 480,
    .x = 0, .y = 0
};

int palette_offset_x = 30;
int palette_offset_y = 20;


void debug_draw(PPU *ppu) {
    // draw palette
    for (uint8_t i = 0; i < 0x20; i++) {
        uint32_t color = get_pixel_color(ppu, i >> 2, i & 0x3);
        int offset_x = i * PALETTE_PIXEL_WIDTH + palette_offset_x;
        for (int y = 0; y < PALETTE_PIXEL_WIDTH; y++) {
            for (int x = 0; x < PALETTE_PIXEL_WIDTH; x++)
                debug_screen_buffer[(palette_offset_y + y) * 256 + (offset_x + x)] = color;
        }
    }

    // draw pattern table
    SDL_UpdateTexture(debug_texture, NULL, (void *)debug_screen_buffer, 256 * sizeof(uint32_t));
    return;
}

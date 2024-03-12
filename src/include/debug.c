#include <SDL2/SDL.h>
#include "debug.h"

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

void debug_draw(PPU *ppu) {
    // draw palette

    // draw pattern table
    return;
}

#ifndef DEBUG_H
#define DEBUG_H

#include <SDL2/SDL.h>
#include "ppu.h"

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 480

extern SDL_Texture *debug_texture;
extern uint32_t debug_screen_buffer[256 * 240];
extern int debug_pixel_format;
extern SDL_Rect debug_rect;
extern SDL_Rect ppu_screen_rect;

void debug_draw(PPU *ppu);

#endif // !DEBUG_H

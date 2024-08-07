#include <stdio.h>
#include <stdlib.h>

#include "../global.h"
#include "../../utils.h"

static void ppu_draw(void);
static Pattern_row get_pattern_row(Byte table_index, Byte plane_num, Byte plane_y);
static uint32_t get_pixel_color(Byte palette_num, Byte pixel);
static void draw_pixel_row(Pattern_row pattern_row, uint32_t *buffer, Byte palette_num, int row_x, int y);
static void update_vram_address(void);

void ppu_clock(void) {
    if (p_ppu->dots >= DOTS) {
        p_ppu->scanlines++;
        if (p_ppu->scanlines >= SCANLINES) {
            p_ppu->scanlines = -1;
            p_ppu->frame_complete = true;
            SDL_UpdateTexture(p_ppu->ppu_draw_texture, NULL, p_ppu->screen_buffer, NES_WIDTH * 4);
        }
        p_ppu->dots = 0;
    }

    if (p_ppu->dots > -1 && p_ppu->dots < NES_WIDTH && p_ppu->scanlines > -1 && p_ppu->scanlines < NES_HEIGHT) 
        ppu_draw();

    if (p_ppu->PPUMASK.Render_background || p_ppu->PPUMASK.Render_sprites) update_vram_address();

    if (p_ppu->scanlines == -1 && p_ppu->dots == 1) {
        p_ppu->PPUSTATUS.Verticle_blank = 0;
    }
    if (p_ppu->scanlines == 241 && p_ppu->dots == 1) {
        p_ppu->PPUSTATUS.Verticle_blank = 1;
        p_ppu->create_nmi = true;
    }
    p_ppu->dots++;
}

uint32_t NES_Palette[64] = {
    0x525252, 0x011A51, 0x0F0F65, 0x230663, 0x36034B, 0x400426, 0x3F0904, 0x321300, 0x1F2000, 0x0B2A00, 0x002F00, 0x002E0A, 0x00262D, 0x000000, 0x000000, 0x000000,
    0xA0A0A0, 0x1E4A9D, 0x3837BC, 0x5828B8, 0x752194, 0x84235C, 0x822E24, 0x6F3F00, 0x515200, 0x316300, 0x1A6B05, 0x0E692E, 0x105C68, 0x000000, 0x000000, 0x000000,
    0xFEFFFF, 0x699EFC, 0x8987FF, 0xAE76FF, 0xCE6DF1, 0xE070B2, 0xDE7C70, 0xC8913E, 0xA6A725, 0x81BA28, 0x63C446, 0x54C17D, 0x56B3C0, 0x3C3C3C, 0x000000, 0x000000,
    0xFEFFFF, 0xBED6FD, 0xCCCCFF, 0xDDC4FF, 0xEAC0F9, 0xF2C1DF, 0xF1C7C2, 0xE8D0AA, 0xD9DA9D, 0xC9E29E, 0xBCE6AE, 0xB4E5C7, 0xB5DFE4, 0xA9A9A9, 0x000000, 0x000000
};

static void ppu_draw(void) {
    // if (-1 < p_ppu->dots && p_ppu->dots < DOTS && -1 < p_ppu->scanlines && p_ppu->scanlines < SCANLINES) {
    //     p_ppu->screen_buffer[p_ppu->scanlines * DOTS + p_ppu->dots] = (rand() % 2) ? 0x00FFFFFF : 0x000000FF;
    // 

    // TODO fill the function

    // DEBUG
    // if (p_ppu->dots && ((p_ppu->dots/* + p_ppu->fine_x*/) % 8)) return;
    if (p_ppu->dots % 8) return;
    /* if (p_ppu->PPUMASK.Render_background) { */
        Byte Plane_num = ppu_read_byte((((Word) p_ppu->scanlines / 8) << 5) | ((Word) p_ppu->dots / 8) | 0x2000);
        // Byte Plane_num = ppu_read_byte((p_ppu->current_address._ & 0xFFF) | 0x2000);
        Pattern_row pattern = get_pattern_row(p_ppu->PPUCTRL.Background_pattern_address, Plane_num, (Byte) (p_ppu->scanlines % 8));
        draw_pixel_row(pattern, p_ppu->screen_buffer, 0, p_ppu->dots, p_ppu->scanlines);
        // p_ppu->dots += 7;
    /* } */
}

void reset_ppu(void) {
    *p_ppu = (PPU) {
        .PPUCTRL._ =  0, .PPUMASK._ =  0,   // ALL registers initialized with 0
        .PPUSTATUS._ =  0,
        .current_address._ = 0, .temp_address._ = 0, .fine_x = 0,
        .PPUDATA = 0,
        .write_latch = 0,               // Write latch to 0
        .VRAM_increment = 1,            // Default VRAM address increment to 1
        .dots = 0, .scanlines = -1,      // Number of dots and scanlines to 0
        .frame_complete = false,        // Frame complete to false
        .create_nmi = false
    };

    memset(p_ppu->screen_buffer, 0, NES_WIDTH * NES_HEIGHT * sizeof(*p_ppu->screen_buffer));
    memset(p_ppu->Bus.Palettes, 0, 0x1F);
    for (int i = 0; i < 4; i++)
        memset(p_ppu->Bus.Nametable[i], 0, 1024);
    p_ppu->PPUSTATUS.Verticle_blank = 1;
}

int init_ppu(SDL_Renderer *renderer) {
    int pixel_format = SDL_PIXELFORMAT_RGB888;
    SDL_Texture *texture = SDL_CreateTexture(renderer,
                                             pixel_format,
                                             SDL_TEXTUREACCESS_STREAMING,
                                             NES_WIDTH, NES_HEIGHT);
    if (texture == NULL)
        ERROR_RETURN("Unable to create NES screen texture\n    SDL error: %s", SDL_GetError());
    p_ppu->ppu_draw_texture = texture;

    int status = SDL_SetRenderTarget(renderer, p_ppu->ppu_draw_texture);
    status = SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    status = SDL_RenderClear(renderer);
    status = SDL_SetRenderTarget(renderer, NULL);

    if (status < 0)
        ERROR_RETURN("Unable to clear NES screen texture\n    SDL error: %s", SDL_GetError());

    return 0;
}

Byte cpu_to_ppu_read(Word address) {
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
            data = (Byte) p_ppu->PPUDATA;
            p_ppu->PPUDATA = ppu_read_byte(p_ppu->current_address._);
            if (p_ppu->current_address._ >= 0x3F00) data = p_ppu->PPUDATA;
            p_ppu->current_address._ += p_ppu->VRAM_increment;
        break;
    }
    return data;
}

Byte cpu_to_ppu_write(Word address, Byte data) {
    address &= 0x0007;
    switch (address) {
        case 0x0: //PPUCTRL *** WRITE only ***
            p_ppu->PPUCTRL._ = data;
            p_ppu->VRAM_increment = (p_ppu->PPUCTRL.VRAM_address_inc) ? 32 : 1;
            p_ppu->temp_address.nametable_select_x = p_ppu->PPUCTRL.Nametable_select_x;
            p_ppu->temp_address.nametable_select_y = p_ppu->PPUCTRL.Nametable_select_y;
        break;

        case 0x1: //PPUMASK *** WRITE only ***
            p_ppu->PPUMASK._ = data;
        break;

        case 0x2: //PPUSTATUS *** READ only ***
        break;

        case 0x3: //OAMADDR *** WRITE only ***
        break;

        case 0x4: //OAMDATA *** READ / WRITE ***
        break;

        case 0x5: //PPUSCROLL *** WRITE only ***
            if (!p_ppu->write_latch) {
                p_ppu->temp_address.coarse_x = data >> 3;
                p_ppu->fine_x = data & 0x7;
                p_ppu->write_latch = 1;
            }
            else {
                p_ppu->temp_address.coarse_y = data >> 3;
                p_ppu->temp_address.fine_y = data & 0x7;
                p_ppu->write_latch = 0;
            }
        break;

        case 0x6: //PPUADDR *** WRITE only ***
            if (!p_ppu->write_latch) {
                p_ppu->temp_address._ = (p_ppu->temp_address._ & 0x00FF) | (((Word) data & 0x3F) << 8);
                p_ppu->write_latch = 1;
            }
            else {
                p_ppu->temp_address._ = (p_ppu->temp_address._ & 0xFF00) | (Word) data;
                p_ppu->current_address._ = p_ppu->temp_address._;
                p_ppu->write_latch = 0;
            }
        break;

        case 0x7: //PPUDATA *** READ / WRITE ***
            p_ppu->PPUDATA = data;
            ppu_write_byte(p_ppu->current_address._, data);
            p_ppu->current_address._ += p_ppu->VRAM_increment;
        break;
    }
    return 0;
}

Byte ppu_read_byte(Word address) {
    Byte data = 0x00;
    address &= 0x3FFF;
    // Inside CHR_ROM or pattern tables
    if (address <= 0x1FFF)
        data = p_mapper->ppu_read(p_mapper, address);
        // Inside Nametable memory
    else if (0x2000 <= address && address <= 0x2FFF) {
        Byte table_index = (address >> 10) & 0x3;
        address &= 0x3FF;
        //Mirroring
        if (p_mapper->mirroring == HORIZONTAL)
            table_index = (table_index & 0x2) ? 2 : 0;
        else
            table_index = (table_index & 0x1) ? 1 : 0;

        data = p_ppu->Bus.Nametable[table_index][address];
    } // Inside Palette memory
    else if (0x3F00 <= address && address <= 0x3FFF) {
        address &= 0x1F;
        address = (address == 0x10) ? 0x00 : address;
        address = (address == 0x14) ? 0x04 : address;
        address = (address == 0x18) ? 0x08 : address;
        address = (address == 0x1C) ? 0x0C : address;
        data = p_ppu->Bus.Palettes[address];
    }

    return data;
}

Byte ppu_write_byte(Word address, Byte data) {
    address &= 0x3FFF;
    // Inside CHR_ROM or pattern tables
    if (address <= 0x1FFF)
        p_mapper->ppu_write(p_mapper, address, data);
        // Inside Nametable memory
    else if (0x2000 <= address && address <= 0x2FFF) {
        Byte table_index = (address >> 10) & 0x3;
        address &= 0x3FF;
        //Mirroring
        if (p_mapper->mirroring == HORIZONTAL)
            table_index = (table_index & 0x2) ? 2 : 0;
        else
            table_index = (table_index & 0x1) ? 1 : 0;

        p_ppu->Bus.Nametable[table_index][address] = data;
    } // Inside Palette memory
    else if (0x3F00 <= address && address <= 0x3FFF) {
        address &= 0x1F;
        address = (address == 0x10) ? 0x00 : address;
        address = (address == 0x14) ? 0x04 : address;
        address = (address == 0x18) ? 0x08 : address;
        address = (address == 0x1C) ? 0x0C : address;
        p_ppu->Bus.Palettes[address] = data;
    }

    return 0;
}

static Pattern_row get_pattern_row(Byte table_index, Byte plane_num, Byte plane_y) {
    Word address = (table_index) ? 0x1000 : 0x0000;
    address |= ((Word) plane_num) << 4;
    address |= (plane_y < 8) ? plane_y : 0;
    return (Pattern_row) {
        .LS_Byte = ppu_read_byte(address),
        .MS_Byte = ppu_read_byte(address | 0x8)
    };
}

static uint32_t get_pixel_color(Byte palette_num, Byte pixel) {
    Byte pixel_index = ppu_read_byte((palette_num << 2) + pixel + 0x3F00);
    return NES_Palette[pixel_index & 0x3F];
}

static void draw_pixel_row(Pattern_row pattern_row, uint32_t *buffer, Byte palette_num, int row_x, int y) {
    for (int i = 7; i > -1; i--) {
        Byte low_bit = pattern_row.LS_Byte & 0x1;
        Byte high_bit = (pattern_row.MS_Byte & 0x1) << 1;
        uint32_t pixel_color = get_pixel_color(palette_num, high_bit | low_bit);
        if ((row_x + i) > -1 && (row_x + i) < 256)
            buffer[(y * NES_WIDTH) + row_x + i] = pixel_color;
        pattern_row.LS_Byte >>= 1;
        pattern_row.MS_Byte >>= 1;
    }
}

static void update_vram_address(void) {
    if (p_ppu->dots == 256) {
        if (p_ppu->current_address.fine_y == 7) {
            if (p_ppu->current_address.coarse_y == 29) {
                p_ppu->current_address.coarse_y = 0;
                p_ppu->current_address.nametable_select_y = ~p_ppu->current_address.nametable_select_y;
            }
            else if (p_ppu->current_address.coarse_y == 31) {
                p_ppu->current_address.coarse_y = 0;
            }
            else {
                p_ppu->current_address.coarse_y++;
            }
            p_ppu->current_address.fine_y = 0;
        }
        else {
            p_ppu->current_address.fine_y++;
        }
    }
    if (p_ppu->dots == 257) {
        p_ppu->current_address.nametable_select_x = p_ppu->temp_address.nametable_select_x;
        p_ppu->current_address.coarse_x = p_ppu->temp_address.coarse_x;
    }
    if (p_ppu->dots > 279 && p_ppu->dots < 305) {
        p_ppu->current_address.fine_y = p_ppu->temp_address.fine_y;
        p_ppu->current_address.nametable_select_y = p_ppu->temp_address.nametable_select_y;
        p_ppu->current_address.coarse_y = p_ppu->temp_address.coarse_y;
    }
    if (p_ppu->dots > 327 || p_ppu->dots < 257) {
        if (p_ppu->dots && !(p_ppu->dots % 8)) {
            if (p_ppu->current_address.coarse_x == 31) {
                p_ppu->current_address.coarse_x = 0;
                p_ppu->current_address.nametable_select_x = ~p_ppu->current_address.nametable_select_x;
            }
            else {
                p_ppu->current_address.coarse_x++;
            }
        }
    }
}

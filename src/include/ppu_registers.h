#ifndef PPU_REGISTERS_H
#define PPU_REGISTERS_H

#include "types.h"

typedef union {
    struct {
        Byte Nametable_address_x : 1;
        Byte Nametable_address_y : 1;
        Byte VRAM_address_inc : 1;
        Byte Sprite_pattern_address : 1;
        Byte Background_pattern_address : 1;
        Byte Sprite_size : 1;
        Byte PPU_select : 1;
        Byte Generate_NMI : 1;
    };
    Byte _;
} PPUCTRL_reg;

typedef union {
    struct {
        Byte Grayscale : 1;
        Byte Show_background_left_8 : 1;
        Byte Show_sprites_left_8 : 1;
        Byte Show_background : 1;
        Byte Show_sprites : 1;
        Byte Emphasize_red : 1;
        Byte Emphasize_green : 1;
        Byte Emphasize_blue : 1;
    };
    Byte _;
} PPUMASK_reg;

typedef union {
    struct {
        Byte PPU_open_bus : 5;
        Byte Sprite_overflow : 1;
        Byte Sprite_zero_hit : 1;
        Byte Verticle_blank : 1;
    };
    Byte _;
} PPUSTATUS_reg;

#endif // !PPU_REGISTERS_H

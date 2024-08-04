#ifndef GLOBAL_H
#define GLOBAL_H

#include "6502/6502.h"
#include "ppu/ppu.h"
#include "cartridge/mapper.h"

extern CPU *p_cpu;
extern PPU *p_ppu;
extern Mapper *p_mapper;

void _set_global_vars(CPU *cpu, PPU *ppu, Mapper *mapper);

#endif // !GLOBAL_H

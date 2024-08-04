#include "global.h"

CPU *p_cpu = NULL;
PPU *p_ppu = NULL;
Mapper *p_mapper = NULL;

void _set_global_vars(CPU *cpu, PPU *ppu, Mapper *mapper) {
    p_cpu = cpu;
    p_ppu = ppu;
    p_mapper = mapper;
}

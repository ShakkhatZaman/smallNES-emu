#ifndef MAPPER_H
#define MAPPER_H

#include <stdint.h>

enum Mappers {
    NROM = 0,
};

enum Mirror_type {
    HORIZONTAL,
    VERTICAL
};

typedef struct Mapper{
    uint16_t mapper_num;
    enum Mirror_type mirroring;
    uint8_t PRG_ROM_banks;
    uint8_t *PRG_ROM_p;
    uint8_t CHR_ROM_banks;
    uint8_t *CHR_ROM_p;
    uint8_t (*cpu_read)(struct Mapper *, uint16_t);
    uint8_t (*cpu_write)(struct Mapper *, uint16_t, uint8_t);
    uint8_t (*ppu_read)(struct Mapper *, uint16_t);
    uint8_t (*ppu_write)(struct Mapper *, uint16_t, uint8_t);
} Mapper;

int load_mapper_functions(Mapper *mapper, uint16_t mapper_num, enum Mirror_type mirror_type);

void _load_NROM(Mapper *mapper);

#endif // MAPPER_H


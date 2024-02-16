#ifndef MAPPER_H
#define MAPPER_H
#include <stdint.h>

typedef struct Mapper{
	uint16_t mapper_num;
	uint8_t PRG_ROM_banks;
	uint8_t *PRG_ROM_p;
	uint8_t CHR_ROM_banks;
	uint8_t *CHR_ROM_p;
	uint8_t (*cpu_read)(struct Mapper *, uint16_t);
	uint8_t (*cpu_write)(struct Mapper *, uint16_t, uint8_t);
	uint8_t (*ppu_read)(struct Mapper *, uint16_t);
	uint8_t (*ppu_write)(struct Mapper *, uint16_t, uint8_t);
} Mapper;

enum Mappers {
	NROM = 0,
};

int load_mapper_functions(Mapper *mapper, uint16_t mapper_num);

void _load_NROM(Mapper *mapper);

#endif // MAPPER_H


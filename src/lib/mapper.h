#ifndef MAPPER_H
#define MAPPER_H
#include <stdint.h>

typedef struct Mapper{
	uint16_t mapper_num;
	uint8_t *PRG_ROM_p;
	uint8_t *CHR_ROM_p;
	uint8_t (*cpu_read)(struct Mapper *, uint16_t);
	uint8_t (*cpu_write)(struct Mapper *, uint16_t, uint8_t);
	uint8_t (*ppu_read)(struct Mapper *, uint16_t);
	uint8_t (*ppu_write)(struct Mapper *, uint16_t, uint8_t);
} Mapper;

void load_mapper_functions(Mapper *mapper, uint16_t mapper_num);

#endif // !MAPPER_H


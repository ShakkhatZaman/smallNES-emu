#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include <stdint.h>
#include "mapper.h"

int load_cartridge(char* filename, Mapper *mapper);

uint16_t get_mapper_num(uint8_t header[]);

uint8_t get_format(uint8_t header[]);

uint64_t get_PRG_ROM_size(uint8_t header[], uint8_t format);

uint64_t get_CHR_ROM_size(uint8_t header[], uint8_t format);

void free_cartridge(Mapper *mapper);

#endif // !CARTRIDGE_H

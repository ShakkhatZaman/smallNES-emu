#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include <stdint.h>
#include "mapper.h"

int load_cartridge(char* filename, Mapper *mapper);

void free_cartridge(Mapper *mapper);

#endif // !CARTRIDGE_H

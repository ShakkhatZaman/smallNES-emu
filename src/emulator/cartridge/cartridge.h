#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include "mapper.h"

int load_cartridge(char* filename);

void free_cartridge(Mapper *mapper);

#endif // !CARTRIDGE_H

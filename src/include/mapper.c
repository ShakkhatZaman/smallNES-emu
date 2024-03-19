#include "mapper.h"
#include <stdio.h>

int load_mapper_functions(Mapper *mapper, uint16_t mapper_num, enum Mirror_type mirror_type) {
	mapper->mapper_num = mapper_num;
    mapper->mirroring = mirror_type;
	switch (mapper_num) {
		case NROM:
			_load_NROM(mapper);
			break;
		default:
			printf("Error: Mapper not found\n");
			return -1;
	}
	return 0;
}

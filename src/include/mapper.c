#include "mapper.h"
#include <stdio.h>

int load_mapper_functions(Mapper *mapper, uint16_t mapper_num) {
	mapper->mapper_num = mapper_num;
	switch (mapper_num) {
		case NROM:
			load_NROM(mapper);
			break;
		default:
			printf("Error: Mapper not found\n");
			return -1;
	}
	return 0;
}

#include "mapper.h"

void load_mapper_functions(Mapper *mapper, uint16_t mapper_num) {
	mapper->mapper_num = mapper_num;
	switch (mapper_num) {
		case NROM:
			load_NROM(mapper);
			break;
	}
}

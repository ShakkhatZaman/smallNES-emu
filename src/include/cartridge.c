#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "cartridge.h"
#include "mapper.h"

int load_cartridge(char* filename, Mapper *mapper){
	FILE *nes_file = fopen(filename, "rb");
	uint8_t header[16];
	
	fread(header, 1, 16, nes_file);
	
	uint16_t Mapper_num = get_mapper_num(header);
	uint8_t format = get_format(header);
	
	int mapper_status = load_mapper_functions(mapper, Mapper_num);
	if (mapper < 0) return -1;

	if (header[6] & 0x04)
		fseek(nes_file, 512, SEEK_CUR);

	uint64_t PRG_ROM_SIZE = get_PRG_ROM_size(header, format, &mapper->PRG_ROM_banks);
	uint64_t CHR_ROM_SIZE = get_CHR_ROM_size(header, format, &mapper->CHR_ROM_banks);

	mapper->PRG_ROM_p = malloc(PRG_ROM_SIZE);
	mapper->CHR_ROM_p = malloc(CHR_ROM_SIZE);

	fread(mapper->PRG_ROM_p, 1, PRG_ROM_SIZE, nes_file);
	fread(mapper->CHR_ROM_p, 1, CHR_ROM_SIZE, nes_file);
	
	fclose(nes_file);

	return 0;
}

uint8_t get_format(uint8_t header[]){
	uint8_t format;
	if (header[0] == 'N' && header[1] == 'E' && header[2] == 'S' && header[3] == 0x1A)
		format = 0; // iNES format

	if (format == 0 && (header[7] & 0x0C) == 0x08)
		format = 1; // NES 2.0 format

	return format;
}

uint16_t get_mapper_num(uint8_t header[]){
	uint16_t Mapper_number = 0;
	Mapper_number |= (header[6] >> 4);
	Mapper_number |= (header[7] & 0xF0);
	Mapper_number |= ((uint16_t) header[8] & 0xF) << 8;
	return Mapper_number;
}

uint64_t get_PRG_ROM_size(uint8_t header[], uint8_t format, uint8_t *num_banks) {
	uint8_t num_PRG_ROM_bank = 0;
	uint64_t PRG_ROM_unit = 0;

	if (format == 1){
		if ((header[9] & 0xF) < 0xF){
			PRG_ROM_unit = 16 * 1024;
			num_PRG_ROM_bank = header[4];
		}
		else {
			num_PRG_ROM_bank = ((header[4] & 0x3) * 2) + 1;
			uint8_t exponent = header[4] >> 2;
			PRG_ROM_unit = exp2(exponent);
		}
	}
	if (format == 0){
		PRG_ROM_unit = 16 * 1024; // 16KB
		num_PRG_ROM_bank = header[4];
	}
	*num_banks = num_PRG_ROM_bank;
	return (uint64_t) (num_PRG_ROM_bank * PRG_ROM_unit);
}

uint64_t get_CHR_ROM_size(uint8_t header[], uint8_t format, uint8_t *num_banks) {
	uint8_t num_CHR_ROM_bank = 0;
	uint64_t CHR_ROM_unit = 0;

	if (format == 1){
		if ((header[9] >> 4) < 0xF){
			CHR_ROM_unit = 8 * 1024;
			num_CHR_ROM_bank = header[5];
		}
		else {
			num_CHR_ROM_bank = ((header[5] & 0x3) * 2) + 1;
			uint8_t exponent = header[5] >> 2;
			CHR_ROM_unit = exp2(exponent);
		}
	}
	if (format == 0){
		CHR_ROM_unit = 8 * 1024;
		num_CHR_ROM_bank = header[5];
	}
	*num_banks = num_CHR_ROM_bank;
	return (uint64_t) (num_CHR_ROM_bank * CHR_ROM_unit);
}

void free_cartridge(Mapper *mapper){
	free(mapper->PRG_ROM_p);
	free(mapper->CHR_ROM_p);
}

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "../../utils.h"
#include "cartridge.h"
#include "mapper.h"

static int _get_format(uint8_t header[]);

static uint16_t _get_mapper_num(uint8_t header[]);

static uint64_t _get_PRG_ROM_size(uint8_t header[], uint8_t format, uint8_t *num_banks);

static uint64_t _get_CHR_ROM_size(uint8_t header[], uint8_t format, uint8_t *num_banks);

int load_cartridge(char* filename, Mapper *mapper){
    FILE *nes_file = fopen(filename, "rb");
    if (nes_file == NULL)
        ERROR_RETURN("Unable to open file: \"%s\"", filename);

    uint8_t header[16];

    fread(header, 1, 16, nes_file);

    int format = _get_format(header);
    if (format < 0)
        ERROR_RETURN("Unable to open file: \"%s\"", "Unknown nesfile format");

    uint16_t Mapper_num = _get_mapper_num(header);
    printf("Mapper number is: %d\n", Mapper_num);

    enum Mirror_type mirroring = (header[6] & 0x1) ? HORIZONTAL : VERTICAL;

    int mapper_status = load_mapper_functions(mapper, Mapper_num, mirroring);
    if (mapper_status < 0) 
        ERROR_RETURN("Unable to load mapper (mapper_num: %d)", Mapper_num);

    if (header[6] & 0x04) fseek(nes_file, 512, SEEK_CUR);

    uint64_t PRG_ROM_SIZE = _get_PRG_ROM_size(header, format, &mapper->PRG_ROM_banks);
    uint64_t CHR_ROM_SIZE = _get_CHR_ROM_size(header, format, &mapper->CHR_ROM_banks);

    mapper->PRG_ROM_p = malloc(PRG_ROM_SIZE);
    if (mapper->PRG_ROM_p == NULL)
        ERROR_RETURN("Unable to allocate space for PRG_ROM (size: %lld)", PRG_ROM_SIZE);

    mapper->CHR_ROM_p = malloc(CHR_ROM_SIZE);
    if (mapper->CHR_ROM_p == NULL)
        ERROR_RETURN("Unable to allocate space for CHR_ROM (size: %lld)", CHR_ROM_SIZE);

    fread(mapper->PRG_ROM_p, 1, PRG_ROM_SIZE, nes_file);
    fread(mapper->CHR_ROM_p, 1, CHR_ROM_SIZE, nes_file);

    fclose(nes_file);

    return 0;
}

static int _get_format(uint8_t header[]){
    int format = -1;
    if (header[0] == 'N' && header[1] == 'E' && header[2] == 'S' && header[3] == 0x1A)
        format = 0; // iNES format

    if (format == 0 && (header[7] & 0x0C) == 0x08)
        format = 1; // NES 2.0 format

    return format;
}

static uint16_t _get_mapper_num(uint8_t header[]){
    uint16_t Mapper_number = 0;
    Mapper_number |= (header[6] >> 4);
    Mapper_number |= (header[7] & 0xF0);
    Mapper_number |= ((uint16_t) header[8] & 0xF) << 8;
    return Mapper_number;
}

static uint64_t _get_PRG_ROM_size(uint8_t header[], uint8_t format, uint8_t *num_banks) {
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

static uint64_t _get_CHR_ROM_size(uint8_t header[], uint8_t format, uint8_t *num_banks) {
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
    if (mapper->PRG_ROM_p != NULL)
        free(mapper->PRG_ROM_p);
    if (mapper->CHR_ROM_p != NULL)
        free(mapper->CHR_ROM_p);
}

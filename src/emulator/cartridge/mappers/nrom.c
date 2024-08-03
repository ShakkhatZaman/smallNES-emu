#include "../mapper.h"
#include <stdint.h>

uint8_t _cpu_read(Mapper *mapper, uint16_t address);
uint8_t _cpu_write(Mapper *mapper, uint16_t address, uint8_t data);
uint8_t _ppu_read(Mapper *mapper, uint16_t address);
uint8_t _ppu_write(Mapper *mapper, uint16_t address, uint8_t data);

void _load_NROM(Mapper *mapper) {
    mapper->cpu_read = _cpu_read;
    mapper->cpu_write = _cpu_write;
    mapper->ppu_read = _ppu_read;
    mapper->ppu_write = _ppu_write;
}

uint8_t _cpu_read(Mapper *mapper, uint16_t address) {
    uint8_t data = 0x00;
    if (address >= 0x8000) {
        // Data mirrored according to bank count
        address &= (mapper->PRG_ROM_banks > 1) ? 0x7FFF : 0x3FFF;
        data = mapper->PRG_ROM_p[address];
    }
    return data;
}

uint8_t _cpu_write(Mapper *mapper, uint16_t address, uint8_t data) {
    //if (address >= 0x8000 && address <= 0xFFFF) {
    //	address &= (mapper->PRG_ROM_banks > 1) ? 0x7FFF : 0x3FFF;
    //	mapper->PRG_ROM_p[address] = data;
    //}
    // TODO
    return 0;
}

uint8_t _ppu_read(Mapper *mapper, uint16_t address) {
    uint8_t data = 0x00;
    if (address <= 0x1FFF)
        data = mapper->CHR_ROM_p[address]; // Pattern tables
    return data;
}

uint8_t _ppu_write(Mapper *mapper, uint16_t address, uint8_t data) {
    // TODO
    return 0;
}

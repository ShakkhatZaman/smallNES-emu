#include <stdio.h>

#include "instructions.h"
#include "../cartridge/cartridge.h"
#include "../global.h"

int64_t *p_total_cycles;

int cpu_clock(void) {
    *p_total_cycles += 1;
    for (int i = 0; i < 3; i++) ppu_clock();
    return 0;
}

void reset_cpu(void) {
    *p_cpu = (CPU) {
        .PC = 0xFFFC,                       // Initializing program counter at 0xFFFC
        .A = 0, .X = 0, .Y = 0,             // All registers to 0
        .D = 0,                             // Setting Decimal flag to 0
        .I = 0,                             // Interrupt Disable to 0
        .C = 0, .V = 0, .N = 0,             // Carry, Overflow and Negative flags set to 0
        .Z = 1,                             // Zero flag to 1 (since Accumulator is 0)
        .SP = 0x01FF,                       // Stack pointer to bottom of stack (0x01FF)
        .temp_byte = 0, .temp_word = 0
    };
    memset(p_cpu->Bus.RAM, 0, 2048);
    memset(p_cpu->Bus.APU_registers, 0, 18);
}

void init_cpu(int64_t *cycles) {
    printf("Initializing CPU...\n");
    p_total_cycles = cycles;
    p_cpu->PC = fetch_word();
}

void execute_cpu_ppu(void) {
    Byte op_code = fetch_byte();
    cpu_clock();
    Ins ins_struct = ins_table[op_code];
    ins_struct.address_mode();
    ins_struct.operation();
    if (p_ppu->create_nmi && p_ppu->PPUCTRL.Generate_NMI) {
        p_ppu->create_nmi = false;
        cpu_nmi();
    }
}

void exit_cpu(void) {
    if (p_mapper)
        free_cartridge(p_mapper);
}


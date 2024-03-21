#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "6502.h"
#include "instructions.h"
#include "cartridge.h"

// DEBUG
#include "logging.h"
#include "ppu.h"

int *p_total_cycles;

int cpu_clock(CPU *cpu) { //main function to process status of cpu
    *p_total_cycles += 1; // 1 clock has passed
    ppu_clock(cpu->p_ppu);
    ppu_clock(cpu->p_ppu);
    ppu_clock(cpu->p_ppu);
    if (cpu->p_ppu->create_nmi && cpu->p_ppu->PPUCTRL.Generate_NMI) {
        cpu->p_ppu->create_nmi = false;
        cpu_nmi(cpu);
    }

    //DEBUG
#ifdef CREATE_LOGS
    LOG_MESSAGE("vert_blank: %d, PPUADDR: %x\n", cpu->p_ppu->PPUSTATUS.Verticle_blank, cpu->p_ppu->PPUADDR);
#endif

    return 0;
}

void reset_cpu(CPU *cpu, CPU_Bus *cpuBus, PPU *p_ppu) {
    *cpu = (CPU) {
        .PC = 0xFFFC,                       // Initializing program counter at 0xFFFC
        .A = 0, .X = 0, .Y = 0,             // All registers to 0
        .D = 0,                             // Setting Decimal flag to 0
        .I = 0,                             // Interrupt Disable to 0
        .C = 0, .V = 0, .N = 0,             // Carry, Overflow and Negative flags set to 0
        .Z = 1,                             // Zero flag to 1 (since Accumulator is 0)
        .SP = 0x01FF,                       // Stack pointer to bottom of stack (0x01FF)
        .p_Bus = cpuBus, .p_ppu = p_ppu,    // Memory pointer to the memory address
        .temp_byte = 0, .temp_word = 0
    };
    memset(cpu->p_Bus->RAM, 0, 2048);
    memset(cpu->p_Bus->APU_registers, 0, 18);
}

void init_cpu(CPU *p_cpu, int *cycles) {
    printf("Initializing CPU...\n");
    p_total_cycles = cycles;
    p_cpu->PC = fetch_word(p_cpu);
}

void load_mapper(CPU *cpu, Mapper *mapper) {
    cpu->p_Bus->mapper = mapper;
    cpu->p_ppu->p_Bus->mapper = mapper;
}

void execute_cpu_ppu(CPU *p_cpu) {
    Byte op_code = fetch_byte(p_cpu);
    cpu_clock(p_cpu);
    Ins ins_struct = ins_table[op_code];
    ins_struct.address_mode(p_cpu);
    ins_struct.operation(p_cpu);

    //DEBUG
#ifdef CREATE_LOGS 
    LOG_MESSAGE("cycles: %d, ins : %x, PC : %x, A : %d, X : %d, Y : %d, tmp_addr: %x\n", *p_total_cycles, op_code, p_cpu->PC, p_cpu->A, p_cpu->X, p_cpu->Y, p_cpu->temp_word);
#endif
}

void exit_cpu(CPU *cpu) {
    if (cpu->p_Bus != NULL)
        free_cartridge(cpu->p_Bus->mapper);
}


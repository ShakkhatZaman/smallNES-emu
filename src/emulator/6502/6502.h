#ifndef CPU_6502_H
#define CPU_6502_H

#include "../../types.h"

#define NOT_ENOUGH_CYCLES 0

#define MEM_SIZE 0x10000        // 65536 B = 1024 * 64 B = 64 kB 

typedef struct {
    Byte RAM[2048];             // 2KB of RAM ->		$0000 - $07FF
    Byte APU_registers[18];     // APU registers ->		$4000 - $4017
} CPU_Bus;

typedef struct CPU {
    Word PC; // Program counter
    Word SP; // Stack pointer

    // Registers
    Byte A, X, Y;

    // Status Flags
    Byte C : 1; // Carry flag
    Byte Z : 1; // Zero flag
    Byte I : 1; // Interrupt Disble
    Byte D : 1; // Decimal mode
    Byte B : 1; // Break command
    Byte V : 1; // Overfloaw flag
    Byte N : 1; // Negative flag

    // Bus
    CPU_Bus Bus;

    // Helper variables
    Byte temp_byte;
    Word temp_word;
    Byte current_mode;
} CPU;

void reset_cpu(void);

void init_cpu(int64_t *cycles);

void execute_cpu_ppu(void);

int cpu_clock(void);

void exit_cpu(void);

#endif //CPU_6502_H

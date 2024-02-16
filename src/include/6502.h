#ifndef CPU_TYPES_H
#define CPU_TYPES_H

#include <stdint.h>
#include "ppu.h"
#include "mapper.h"

#define NOT_ENOUGH_CYCLES 0

enum {
	NO_INS, INF_CYCLES
};

#define MEM_SIZE 0x10000 // 65536 B = 1024 * 64 B = 64 kB 

typedef uint8_t Byte;
typedef uint16_t Word;

typedef struct {
	Byte RAM[2048]; 			// 2KB of RAM ->		$0000 - $07FF
	Byte APU_registers[18]; 	// APU registers ->		$4000 - $4017
	Mapper *mapper;
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
	
	// Bus pointer
	CPU_Bus *p_Bus;

	// PPU pointer
	PPU *p_ppu;

	// Helper variables
	Byte temp_byte;
	Word temp_word;
	Byte current_mode;
} CPU;

void reset(CPU *cpu, CPU_Bus *cpuBus, PPU *p_PPU, PPU_Bus *ppu_bus);

void init_cpu(CPU *p_cpu, int *cycles);

void execute(CPU *p_cpu);

int cpu_clock(CPU *cpu);

void load_mapper_to_cpu(CPU * cpu, Mapper * mapper);

void exit_cpu(CPU *cpu, int argc);

#endif //CPU_TYPES_H

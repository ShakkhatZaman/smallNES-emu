#ifndef CPU_TYPES_H
#define CPU_TYPES_H

#include <stdint.h>

#define NOT_ENOUGH_CYCLES 0

enum {
	NO_INS, INF_CYCLES
};

#define MEM_SIZE 0x10000 // 65536 B = 1024 * 64 B = 64 kB 

typedef uint8_t Byte;
typedef uint16_t Word;

typedef struct Mem {
	Byte Data[MEM_SIZE];
} Mem;

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
	
	// Memory pointer
	struct Mem *pMem;

	// Helper variables
	Byte temp_byte;
	Word temp_word;
	Byte current_mode;
} CPU;

void reset(CPU *cpu, struct Mem *pM);

void execute(CPU *cpu, int cycles);

int execute_instruction(CPU *cpu, Byte ins, int cycles, Byte decr);

#endif

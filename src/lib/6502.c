#include <stdio.h>
#include "6502.h"
#include "instructions.h"
#include "ppu.h"
#include "mapper.h"
#include "cartridge.h"

Byte cycle_dec;
int total_cycles;

int cpu_clock(CPU *cpu){ //main function to check status of cpu
	if (!total_cycles){
		return 0; // exit emulator
	}
	total_cycles -= cycle_dec; // 1 clock has passed
	return 0;
}

void reset(CPU *cpu, CPU_Bus *cpuBus, PPU *p_PPU, PPU_Bus *ppu_bus){
	cpu->PC = 0xFFFC; 				// Initializing program counter at 0xFFFC
	cpu->A = cpu->X = cpu->Y = 0;	// All registers to 0
	cpu->D = 0;						// Setting Decimal flag to 0
	cpu->I = 1;						// Interrupt Disable to 0
	cpu->C = cpu->V = cpu->N = 0;	// Carry, Overflow and Negative flags set to 0
	cpu->Z = 1;						// Zero flag to 1 (since Accumulator is 0)
	cpu->SP = 0x01FF;				// Stack pointer to bottom of stack (0x01FF)
	cpu->p_Bus = cpuBus;			// Memory pointer to the memory address
	cpu->p_ppu = p_PPU;
	cpu->p_ppu->p_Bus = ppu_bus;
	cpu->temp_byte = cpu->temp_word = 0;
}

void load_mapper_to_cpu(CPU * cpu, Mapper * mapper){
	cpu->p_Bus->mapper = mapper;
	cpu->p_ppu->p_Bus->mapper = mapper;
}

void execute(CPU *p_cpu, int cycles){
	Byte op_code = NO_INS;
	total_cycles = cycles;
	cycle_dec = (total_cycles < 0) ? 0 : 1;
	p_cpu->PC = fetch_word(p_cpu);
	while (cycles != 0){
		op_code = fetch_byte(p_cpu);
		cpu_clock(p_cpu);
		execute_instruction(p_cpu, op_code);
	}
	printf("Exiting after istruction %x", op_code);
}

int execute_instruction(CPU *cpu, Byte op_code){
	Ins ins_struct = ins_table[op_code];
	Byte additional_cycles = ins_struct.address_mode(cpu);
	ins_struct.operation(cpu);
	return 0;
}

Byte load_cartrigde_to_cpu(CPU *cpu){
	return 0; //placeholder
}

void exit_cpu(CPU *cpu, int argc){
	if (argc > 1)
		free_cartridge(cpu->p_Bus->mapper);
	return;
}


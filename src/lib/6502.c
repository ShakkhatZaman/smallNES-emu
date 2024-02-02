#include "6502.h"
#include "instructions.h"
#include "ppu.h"
#include "mapper.h"
#include "cartridge.h"

void reset(CPU *cpu, CPU_Bus *cpuBus, PPU *p_PPU, PPU_Bus *ppu_bus){
	cpu->PC = 0xFFFC; 				// Initializing program counter at 0xFFFC
	cpu->A = cpu->X = cpu->Y = 0;	// All registers to 0
	cpu->D = 0;						// Setting Decimal flag to 0
	cpu->I = 1;						// Interrupt Disable to 0
	cpu->C = cpu->V = cpu->N = 0;	// Carry, Overflow and Negative flags set to 0
	cpu->Z = 1;						// Zero flag to 1 (since Accumulator is 0)
	cpu->SP = 0x01FF;				// Stack pointer to bottom of stack (0x01FF)
	cpu->p_Bus = cpuBus;					// Memory pointer to the memory address
	cpu->p_ppu = p_PPU;
	cpu->p_ppu->p_Bus = ppu_bus;
	cpu->temp_byte = cpu->temp_word = 0;
}

void load_mapper_to_cpu(CPU * cpu, Mapper * mapper){
	cpu->p_Bus->mapper = mapper;
	cpu->p_ppu->p_Bus->mapper = mapper;
}

void execute(CPU *p_cpu, int cycles){
	Byte Instruction = NO_INS;
	Byte decrement_value = (cycles < 0) ? 0 : 1;
	if (cycles < 2 && cycles > -1) return;
	p_cpu->PC = fetch_word(p_cpu);
	cycles -= (decrement_value * 2);
	while (cycles != 0){
		if (cycles < 1 && cycles > -1) return;
		Instruction = fetch_byte(p_cpu);
		cycles = execute_instruction(p_cpu, Instruction, cycles, decrement_value);
	}
}

int execute_instruction(CPU *cpu, Byte ins, int cycles, Byte decr){
	Ins ins_struct = ins_table[ins];
	Byte cycle_count = ins_struct.max_cycle_count;
	if ((cycles < cycle_count) && (cycles > -1)) return 0;

	Byte additional_cycles = ins_struct.address_mode(cpu);
	ins_struct.operation(cpu);
	cycles -= (decr * ins_struct.min_cycle_count);
	cycles -= (decr * additional_cycles);
	return cycles;
}

Byte load_cartrigde_to_cpu(CPU *cpu){
	return 0; //placeholder
}

void exit_cpu(CPU *cpu, int argc){
	if (argc > 1)
		free_cartridge(cpu->p_Bus->mapper);
	return;
}

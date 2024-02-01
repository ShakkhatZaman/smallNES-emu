#include "6502.h"
#include "instructions.h"

void reset(CPU *cpu, struct Mem *pM){
	cpu->PC = 0xFFFC; 				// Initializing program counter at 0xFFFC
	cpu->A = cpu->X = cpu->Y = 0;	// All registers to 0
	cpu->D = 0;						// Setting Decimal flag to 0
	cpu->I = 1;						// Interrupt Disable to 0
	cpu->C = cpu->V = cpu->N = 0;	// Carry, Overflow and Negative flags set to 0
	cpu->Z = 1;						// Zero flag to 1 (since Accumulator is 0)
	cpu->SP = 0x01FF;				// Stack pointer to bottom of stack (0x01FF)
	cpu->pMem = pM;					// Memory pointer to the memory address
	cpu->temp_byte = cpu->temp_word = 0;
}

void execute(CPU *p_cpu, int cycles){
	Byte Instruction = NO_INS;
	Byte decrement_value = (cycles < 0) ? 0 : 1;
	while (cycles != 0){
		if (cycles < 1 && cycles > -1) return;
		Instruction = fetch_byte(p_cpu);
		cycles -= decrement_value;
		cycles = execute_instruction(p_cpu, Instruction, cycles, decrement_value);
	}
}

int execute_instruction(CPU *cpu, Byte ins, int cycles, Byte decr){
	Ins ins_struct = ins_table[ins];
	Byte cycle_count = ins_struct.max_cycle_count;
	if ((cycles < cycle_count) && (cycles > -1))
		return 0;

	Byte additional_cycles = ins_struct.address_mode(cpu);
	ins_struct.operation(cpu);
	cycles -= (decr * ins_struct.min_cycle_count);
	cycles -= (decr * additional_cycles);
	return cycles;
}

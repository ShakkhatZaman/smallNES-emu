#include "6502.h"
#include "instructions.h"

Byte fetch_byte(CPU *cpu){
	Word counter = cpu->PC;
	cpu->PC = (counter == 0xFFFF) ? 0 : counter + 1;
	//printf("%x\n", cpu->PC);
	return cpu->pMem->Data[counter];
}

#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include "6502.h"

#define NULL_INS {NULL_M, NULL_OP, 0, 0}

/*Instruction struct*/
typedef struct{
	Byte (*address_mode)(CPU *);
	Byte (*operation)(CPU *);
	Byte min_cycle_count;
	Byte max_cycle_count;
} Ins;

extern const Ins ins_table[256];

/* Helper fucntions */

Byte fetch_byte(CPU *cpu);

void set_status_A(CPU *cpu);

Word fetch_word(CPU *cpu);

void stack_push(CPU *cpu, Byte data);

Byte stack_pop(CPU *cpu);

Byte read_byte(CPU *cpu, Word address);

Byte write_byte(CPU *cpu, Word address, Byte data);

/*Addressing modes*/

Byte ABS(CPU *cpu);

Byte ABX(CPU *cpu);

Byte ABY(CPU *cpu);

Byte ACC(CPU *cpu);

Byte IMP(CPU *cpu);

Byte IMM(CPU *cpu);

Byte IND(CPU *cpu);

Byte IZX(CPU *cpu);

Byte IZY(CPU *cpu);

Byte REL(CPU *cpu);

Byte ZP0(CPU *cpu);

Byte ZPX(CPU *cpu);

Byte ZPY(CPU *cpu);

Byte NULL_M(CPU *cpu);

/*Operation functions*/

Byte ADC(CPU *cpu);

Byte AND(CPU *cpu);

Byte ASL(CPU *cpu);

Byte BCC(CPU *cpu);

Byte BCS(CPU *cpu);

Byte BEQ(CPU *cpu);

Byte BIT(CPU *cpu);

Byte BMI(CPU *cpu);

Byte BNE(CPU *cpu);

Byte BPL(CPU *cpu);

Byte BRK(CPU *cpu);

Byte BVC(CPU *cpu);

Byte BVS(CPU *cpu);

Byte CLC(CPU *cpu);

Byte CLD(CPU *cpu);

Byte CLI(CPU *cpu);

Byte CLV(CPU *cpu);

Byte CMP(CPU *cpu);

Byte CPX(CPU *cpu);

Byte CPY(CPU *cpu);

Byte DEC(CPU *cpu);

Byte DEX(CPU *cpu);

Byte DEY(CPU *cpu);

Byte EOR(CPU *cpu);

Byte INC(CPU *cpu);

Byte INX(CPU *cpu);

Byte INY(CPU *cpu);

Byte JMP(CPU *cpu);

Byte JSR(CPU *cpu);

Byte LDA(CPU *cpu);

Byte LDX(CPU *cpu);

Byte LDY(CPU *cpu);

Byte LSR(CPU *cpu);

Byte NOP(CPU *cpu);

Byte ORA(CPU *cpu);

Byte PHA(CPU *cpu);

Byte PHP(CPU *cpu);

Byte PLA(CPU *cpu);

Byte PLP(CPU *cpu);

Byte ROL(CPU *cpu);

Byte ROR(CPU *cpu);

Byte RTI(CPU *cpu);

Byte RTS(CPU *cpu);

Byte SBC(CPU *cpu);

Byte SEC(CPU *cpu);

Byte SED(CPU *cpu);

Byte SEI(CPU *cpu);

Byte STA(CPU *cpu);

Byte STX(CPU *cpu);

Byte STY(CPU *cpu);

Byte TAX(CPU *cpu);

Byte TAY(CPU *cpu);

Byte TSX(CPU *cpu);

Byte TXA(CPU *cpu);

Byte TXS(CPU *cpu);

Byte TYA(CPU *cpu);

Byte NULL_OP(CPU *cpu);

#endif

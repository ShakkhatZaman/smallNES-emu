#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include "../../types.h"

#define NULL_INS {NUL, NUL}

/*Instruction struct*/
typedef struct{
    Byte (*address_mode)(void);
    Byte (*operation)(void);
} Ins;

extern const Ins ins_table[256];

/* Helper fucntions */

Byte fetch_byte(void);

Word fetch_word(void);

void cpu_irq(void);

void cpu_nmi(void);

/*Addressing modes*/

Byte ABS(void);

Byte ABX(void);

Byte ABY(void);

Byte ACC(void);

Byte IMP(void);

Byte IMM(void);

Byte IND(void);

Byte IZX(void);

Byte IZY(void);

Byte REL(void);

Byte ZP0(void);

Byte ZPX(void);

Byte ZPY(void);

/*Operation functions*/

Byte ADC(void);

Byte AND(void);

Byte ASL(void);

Byte BCC(void);

Byte BCS(void);

Byte BEQ(void);

Byte BIT(void);

Byte BMI(void);

Byte BNE(void);

Byte BPL(void);

Byte BRK(void);

Byte BVC(void);

Byte BVS(void);

Byte CLC(void);

Byte CLD(void);

Byte CLI(void);

Byte CLV(void);

Byte CMP(void);

Byte CPX(void);

Byte CPY(void);

Byte DEC(void);

Byte DEX(void);

Byte DEY(void);

Byte EOR(void);

Byte INC(void);

Byte INX(void);

Byte INY(void);

Byte JMP(void);

Byte JSR(void);

Byte LDA(void);

Byte LDX(void);

Byte LDY(void);

Byte LSR(void);

Byte NOP(void);

Byte ORA(void);

Byte PHA(void);

Byte PHP(void);

Byte PLA(void);

Byte PLP(void);

Byte ROL(void);

Byte ROR(void);

Byte RTI(void);

Byte RTS(void);

Byte SBC(void);

Byte SEC(void);

Byte SED(void);

Byte SEI(void);

Byte STA(void);

Byte STX(void);

Byte STY(void);

Byte TAX(void);

Byte TAY(void);

Byte TSX(void);

Byte TXA(void);

Byte TXS(void);

Byte TYA(void);

Byte NUL(void);

#endif

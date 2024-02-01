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

#endif

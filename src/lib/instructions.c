#include <stdint.h>
#include "6502.h"
#include "instructions.h"

/*Helper functions */

Byte fetch_byte(CPU *cpu){
	Word counter = cpu->PC;
	cpu->PC = (counter == 0xFFFF) ? 0 : counter + 1;
	//printf("%x\n", cpu->PC);
	return cpu->pMem->Data[counter];
}

Word fetch_word(CPU *cpu){
	Word counter = cpu->PC;
	cpu->PC = (counter == 0xFFFF) ? 0 : counter + 1;
	Word Data = cpu->pMem->Data[counter];

	counter = cpu->PC;
	cpu->PC = (counter == 0xFFFF) ? 0 : counter + 1;
	Data |= (((Word) cpu->pMem->Data[counter]) << 8);
	return Data;
}

Byte read_byte(CPU *cpu, Word address){
	return cpu->pMem->Data[address];
}

Byte write_byte(CPU *cpu, Word address, Byte data){
	cpu->pMem->Data[address] = data;
	return 0;
}

void stack_push(CPU *cpu, Byte data){
	cpu->pMem->Data[cpu->SP] = data;
	cpu->SP = (cpu->SP == 0x0100) ? 0x01FF : cpu->SP - 1;
}

Byte stack_pop(CPU *cpu){
	cpu->SP = (cpu->SP == 0x01FF) ? 0x0100 : cpu->SP + 1;
	return cpu->pMem->Data[cpu->SP];
}

void set_status_A(CPU *cpu){
	cpu->Z = (cpu->A == 0); // Zero flag is set if A is Zero
	cpu->N = (cpu->A & 0b10000000) ? 1 : 0;

}

/*Addressing modes*/

Byte ABS(CPU *cpu){
	Word data_address = fetch_word(cpu);
	cpu->temp_word = data_address;
	cpu->temp_byte = read_byte(cpu, data_address);
	cpu->current_mode = 1;
	return 0;
}

Byte ABX(CPU *cpu){
	Word data_address = fetch_word(cpu);
	Word data_address_x = data_address + cpu->X;
	cpu->temp_word = data_address_x;
	cpu->temp_byte = read_byte(cpu, data_address_x);
	cpu->current_mode = 2;
	if ((data_address & 0xFF00) == (data_address_x & 0xFF00))
		return 0;
	else
		return 1;
}

Byte ABY(CPU *cpu){
	Word data_address = fetch_word(cpu);
	Word data_address_y = data_address + cpu->Y;
	cpu->temp_word = data_address_y;
	cpu->temp_byte = read_byte(cpu, data_address_y);
	cpu->current_mode = 3;
	if ((data_address & 0xFF00) == (data_address_y & 0xFF00))
		return 0;
	else
		return 1;
}

Byte ACC(CPU *cpu){
	cpu->temp_byte = cpu->A;
	cpu->current_mode = 4;
	return 0;
}

Byte IMP(CPU *cpu){
	cpu->current_mode = 5;
	return 0;
}

Byte IMM(CPU *cpu){
	cpu->temp_byte = fetch_byte(cpu);
	cpu->current_mode = 6;
	return 0;
}

Byte IND(CPU *cpu){
	Word data_address = fetch_word(cpu);
	Byte byte_data = read_byte(cpu, data_address);
	Word word_data;

	if ((data_address & 0x00FF) == 0x00FF)
		word_data = (((Word) read_byte(cpu, data_address & 0xFF00)) << 8) | ((Word) byte_data);

	else 
		word_data = (((Word) read_byte(cpu, data_address + 1)) << 8) | ((Word) byte_data);

	cpu->temp_word = word_data;
	cpu->temp_byte = read_byte(cpu, word_data);
	cpu->current_mode = 7;
	return 0;
}

Byte IZX(CPU *cpu){
	Byte zp_data_address_x = fetch_byte(cpu);
	zp_data_address_x += cpu->X;
	Word full_zp_address_x = (Word) zp_data_address_x;
	Byte byte_data = read_byte(cpu, full_zp_address_x);
	Word word_data = (((Word) read_byte(cpu, full_zp_address_x + 1)) << 8) | ((Word) byte_data);
	cpu->temp_word = word_data;
	cpu->temp_byte = read_byte(cpu, word_data);
	cpu->current_mode = 8;
	return 0;
}

Byte IZY(CPU *cpu){
	Byte zp_data_address_y = fetch_byte(cpu);
	Word full_zp_address_y = (Word) zp_data_address_y;
	Byte byte_data = read_byte(cpu, full_zp_address_y);
	Word word_data = (((Word) read_byte(cpu, full_zp_address_y + 1)) << 8) | ((Word) byte_data);
	Word temp_word_data = (Word) cpu->Y + word_data;
	cpu->temp_word = word_data;
	cpu->temp_byte = read_byte(cpu, word_data);
	cpu->current_mode = 9;
	if ((temp_word_data & 0xFF00) == (word_data & 0xFF00))
		return 0;
	else
		return 1;
}

Byte REL(CPU *cpu){
	Byte byte_offset = fetch_byte(cpu);
	cpu->temp_byte = byte_offset;
	Word target = cpu->PC;
	target += (int8_t) byte_offset;

	cpu->temp_word = target;
	cpu->current_mode = 10;
	
	if ((cpu->PC & 0xFF00) == (target & 0xFF00))
		return 1;
	else
		return 2;
}

Byte ZP0(CPU *cpu){
	Byte zero_page_address = fetch_byte(cpu);
	cpu->temp_word = (Word) zero_page_address;
	cpu->temp_byte = read_byte(cpu, (Word) zero_page_address);
	cpu->current_mode = 11;
	return 0;
}

Byte ZPX(CPU *cpu){
	Byte zero_page_address_x = fetch_byte(cpu);
	zero_page_address_x += cpu->X;
	cpu->temp_word = (Word) zero_page_address_x;
	cpu->temp_byte = read_byte(cpu, (Word) zero_page_address_x);
	cpu->current_mode = 12;
	return 0;
}

Byte ZPY(CPU *cpu){
	Byte zero_page_address_y = fetch_byte(cpu);
	zero_page_address_y += cpu->Y;
	cpu->temp_word = (Word) zero_page_address_y;
	cpu->temp_byte = read_byte(cpu, (Word) zero_page_address_y);
	cpu->current_mode = 13;
	return 0;
}

Byte NULL_M(CPU *cpu){
	return 0;
}

/*Operation functions*/

Byte ADC(CPU *cpu){
	Word acc = (Word) cpu->A;
	Word tmp_byte = (Word) cpu->temp_byte;
	Word temp_sum = acc + tmp_byte + cpu->C;

	cpu->A = (Byte) temp_sum;
	cpu->C = (temp_sum > 256) ? 1 : 0;
	cpu->Z = (temp_sum == 0) ? 1 : 0;
	cpu->V = (~(acc ^ tmp_byte) & (acc ^ temp_sum) & 0x80) ? 1 : 0;
	cpu->N = (temp_sum & 0x80) ? 1 : 0;
	return 0;
}

Byte AND(CPU *cpu){
	cpu->A &= cpu->temp_byte;
	cpu->Z = (cpu->A == 0) ? 1 : 0;
	cpu->N = (cpu->A & 0x80) >> 7;
	return 0;
}

Byte ASL(CPU *cpu){
	if (cpu->current_mode == 4){
		cpu->C = (cpu->A & 0x80) ? 1 : 0;
		cpu->A = cpu->A << 1;
		cpu->Z = (cpu->A == 0) ? 1 : 0;
		cpu->N = (cpu->A & 0x80) ? 1 : 0;
	}
	else {
		cpu->C = (cpu->temp_byte & 0x80) ? 1 : 0;
		cpu->temp_byte >>= 1;
		cpu->Z = (cpu->temp_byte == 0) ? 1 : 0;
		cpu->N = (cpu->temp_byte & 0x80) ? 1 : 0;
		write_byte(cpu, cpu->temp_word, cpu->temp_byte);
	}
	return 0;
}

Byte BCC(CPU *cpu){
	if (!cpu->C)
		cpu->PC = cpu->temp_word;
	return 0;
}

Byte BCS(CPU *cpu){
	if (cpu->C)
		cpu->PC = cpu->temp_word;
	return 0;
}

Byte BEQ(CPU *cpu){
	if (cpu->Z)
		cpu->PC = cpu->temp_word;
	return 0;
}

Byte BIT(CPU *cpu){
	cpu->N = (cpu->temp_byte & 0b10000000) ? 1 : 0;
	cpu->V = (cpu->temp_byte & 0b01000000) ? 1 : 0;
	cpu->Z = (cpu->temp_byte & cpu->A) ? 0 : 1;
	return 0;
}

Byte BMI(CPU *cpu){
	if (cpu->N)
		cpu->PC = cpu->temp_word;
	return 0;
}

Byte BNE(CPU *cpu){
	if (!cpu->Z)
		cpu->PC = cpu->temp_word;
	return 0;
}

Byte BPL(CPU *cpu){
	if (!cpu->N)
		cpu->PC = cpu->temp_word;
	return 0;
}

Byte BRK(CPU *cpu){
	Word target = cpu->PC + 1;
	stack_push(cpu, (Byte) cpu->PC >> 8);
	stack_push(cpu, (Byte) cpu->PC);
	cpu->I = 1;
	Byte SR;
	SR |= (cpu->N) ? 0x80 : 0;
	SR |= (cpu->V) ? 0x64 : 0;
	SR |= (cpu->B) ? 0x16 : 0;
	SR |= (cpu->D) ? 0x8 : 0;
	SR |= (cpu->I) ? 0x4 : 0;
	SR |= (cpu->Z) ? 0x2 : 0;
	SR |= (cpu->C) ? 0x1 : 0;
	stack_push(cpu, SR);
	return 0;
}

Byte BVC(CPU *cpu){
	if (!cpu->V)
		cpu->PC = cpu->temp_word;
	return 0;
}

Byte BVS(CPU *cpu){
	if (cpu->V)
		cpu->PC = cpu->temp_word;
	return 0;
}

Byte CLC(CPU *cpu){
	cpu->C = 0;
	return 0;
}

Byte CLD(CPU *cpu){
	cpu->D = 0;
	return 0;
}

Byte CLI(CPU *cpu){
	cpu->I = 0;
	return 0;
}

Byte CLV(CPU *cpu){
	cpu->V = 0;
	return 0;
}

Byte CMP(CPU *cpu){
	Word result = (Word) cpu->A;
	Word mem = (Word) cpu->temp_byte;
	result += (~mem + 1);
	cpu->C = (result > 256) ? 1 : 0;
	cpu->Z = (result == 0) ? 1 : 0;
	cpu->N = (result & 0x80) ? 1 : 0;
	return 0;
}

Byte CPX(CPU *cpu){
	Word result = (Word) cpu->X;
	Word mem = (Word) cpu->temp_byte;
	result += (~mem + 1);
	cpu->C = (result > 256) ? 1 : 0;
	cpu->Z = (result == 0) ? 1 : 0;
	cpu->N = (result & 0x80) ? 1 : 0;
	return 0;
}

Byte CPY(CPU *cpu){
	Word result = (Word) cpu->Y;
	Word mem = (Word) cpu->temp_byte;
	result += (~mem + 1);
	cpu->C = (result > 256) ? 1 : 0;
	cpu->Z = (result == 0) ? 1 : 0;
	cpu->N = (result & 0x80) ? 1 : 0;
	return 0;
}

Byte DEC(CPU *cpu){
	cpu->temp_byte -= 1; 
	write_byte(cpu, cpu->temp_word, cpu->temp_byte);
	cpu->Z = (cpu->temp_byte == 0) ? 1 : 0;
	cpu->N = (cpu->temp_byte & 0x80) ? 1 : 0;
	return 0;
}

Byte DEX(CPU *cpu){
	cpu->X -= 1; 
	cpu->Z = (cpu->X == 0) ? 1 : 0;
	cpu->N = (cpu->X & 0x80) ? 1 : 0;
	return 0;
}

Byte DEY(CPU *cpu){
	cpu->Y -= 1; 
	cpu->Z = (cpu->Y == 0) ? 1 : 0;
	cpu->N = (cpu->Y & 0x80) ? 1 : 0;
	return 0;
}

Byte EOR(CPU *cpu){
	cpu->A ^= cpu->temp_byte;
	cpu->Z = (cpu->A == 0) ? 1 : 0;
	cpu->N = (cpu->A & 0x80) ? 1 : 0;
	return 0;
}

Byte INC(CPU *cpu){
	cpu->temp_byte += 1; 
	write_byte(cpu, cpu->temp_word, cpu->temp_byte);
	cpu->Z = (cpu->temp_byte == 0) ? 1 : 0;
	cpu->N = (cpu->temp_byte & 0x80) ? 1 : 0;
	return 0;
}

Byte INX(CPU *cpu){
	cpu->X += 1; 
	cpu->Z = (cpu->X == 0) ? 1 : 0;
	cpu->N = (cpu->X & 0x80) ? 1 : 0;
	return 0;
}

Byte INY(CPU *cpu){
	cpu->Y += 1; 
	cpu->Z = (cpu->Y == 0) ? 1 : 0;
	cpu->N = (cpu->Y & 0x80) ? 1 : 0;
	return 0;
}

Byte JMP(CPU *cpu){
	cpu->PC = cpu->temp_word;
	return 0;
}

Byte JSR(CPU *cpu){
	Word current_pc = cpu->PC - 1;
	stack_push(cpu, (Byte) (current_pc >> 8));
	stack_push(cpu, (Byte) current_pc);
	cpu->PC = cpu->temp_word;
	return 0;
}

Byte LDA(CPU *cpu){
	cpu->A = cpu->temp_byte;
	set_status_A(cpu);
	return 0;
}

Byte LDX(CPU *cpu){
	cpu->X = cpu->temp_byte;
	cpu->Z = (cpu->X == 0) ? 1 : 0;
	cpu->N = (cpu->X & 0x80) ? 1 : 0;
	return 0;
}

Byte LDY(CPU *cpu){
	cpu->Y = cpu->temp_byte;
	cpu->Z = (cpu->Y == 0) ? 1 : 0;
	cpu->N = (cpu->Y & 0x80) ? 1 : 0;
	return 0;
}

Byte LSR(CPU *cpu){
	if (cpu->current_mode == 4){
		cpu->C = (cpu->A & 1) ? 1 : 0;
		cpu->A = cpu->A >> 1;
		cpu->Z = (cpu->A == 0) ? 1 : 0;
		cpu->N = (cpu->A & 0x80) ? 1 : 0;
	}
	else {
		cpu->C = (cpu->temp_byte & 1) ? 1 : 0;
		cpu->temp_byte >>= 1;
		cpu->Z = (cpu->temp_byte == 0) ? 1 : 0;
		cpu->N = (cpu->temp_byte & 0x80) ? 1 : 0;
		write_byte(cpu, cpu->temp_word, cpu->temp_byte);
	}
	return 0;
}

Byte NOP(CPU *cpu){
	return 0;
}

Byte ORA(CPU *cpu){
	cpu->A |= cpu->temp_byte;
	cpu->Z = (cpu->A == 0) ? 1 : 0;
	cpu->N = (cpu->A & 0x80) ? 1 : 0;
	return 0;
}

Byte PHA(CPU *cpu){
	stack_push(cpu, cpu->A);
	return 0;
}

Byte PHP(CPU *cpu){
	Byte SR;
	SR |= (cpu->N) ? 0x80 : 0;
	SR |= (cpu->V) ? 0x64 : 0;
	SR |= 0x64;
	SR |= (cpu->B) ? 0x16 : 0;
	SR |= (cpu->D) ? 0x8 : 0;
	SR |= 0x4; // Interrupt flag set to 1
	SR |= (cpu->Z) ? 0x2 : 0;
	SR |= (cpu->C) ? 0x1 : 0;
	stack_push(cpu, SR);
	return 0;
}

Byte PLA(CPU *cpu){
	cpu->A = stack_pop(cpu);
	cpu->Z = (cpu->A == 0) ? 1 : 0;
	cpu->N = (cpu->A & 0x80) ? 1 : 0;
	return 0;
}

Byte PLP(CPU *cpu){
	Byte PS = stack_pop(cpu);
	cpu->N = (PS & 0x80) ? 1 : 0;
	cpu->V = (PS & 0x64) ? 1 : 0;
	cpu->B = (PS & 0x16) ? 1 : 0;
	cpu->D = (PS & 0x8) ? 1 : 0;
	cpu->I = (PS & 0x4) ? 1 : 0;
	cpu->Z = (PS & 0x2) ? 1 : 0;
	cpu->C = (PS & 1) ? 1 : 0;
	return 0;
}

Byte ROL(CPU *cpu){
	if (cpu->current_mode == 4){
		cpu->C = (cpu->A & 0x80) ? 1 : 0;
		cpu->A <<= 1;
		cpu->A |= cpu->C;
		cpu->Z = (cpu->A == 0) ? 1 : 0;
		cpu->N = (cpu->A & 0x80) ? 1 : 0;
	}
	else {
		cpu->C = (cpu->temp_byte & 0x80) ? 1 : 0;
		cpu->temp_byte <<= 1;
		cpu->temp_byte |= cpu->C;
		cpu->Z = (cpu->temp_byte == 0) ? 1 : 0;
		cpu->N = (cpu->temp_byte & 0x80) ? 1 : 0;
		write_byte(cpu, cpu->temp_word, cpu->temp_byte);
	}
	return 0;
}

Byte ROR(CPU *cpu){
	if (cpu->current_mode == 4){
		cpu->C = (cpu->A & 1) ? 1 : 0;
		cpu->A >>= 1;
		cpu->A |= (cpu->C) ? 0x80 : 0;
		cpu->Z = (cpu->A == 0) ? 1 : 0;
		cpu->N = (cpu->A & 0x80) ? 1 : 0;
	}
	else {
		cpu->C = (cpu->temp_byte & 1) ? 1 : 0;
		cpu->temp_byte >>= 1;
		cpu->temp_byte |= (cpu->C) ? 0x80 : 0;
		cpu->Z = (cpu->temp_byte == 0) ? 1 : 0;
		cpu->N = (cpu->temp_byte & 0x80) ? 1 : 0;
		write_byte(cpu, cpu->temp_word, cpu->temp_byte);
	}
	return 0;
}

Byte RTI(CPU *cpu){
	Byte PS = stack_pop(cpu);
	cpu->N = (PS & 0x80) ? 1 : 0;
	cpu->V = (PS & 0x64) ? 1 : 0;
	cpu->B = (PS & 0x16) ? 1 : 0;
	cpu->D = (PS & 0x8) ? 1 : 0;
	cpu->I = (PS & 0x4) ? 1 : 0;
	cpu->Z = (PS & 0x2) ? 1 : 0;
	cpu->C = (PS & 1) ? 1 : 0;
	Word new_PC = (Word) stack_pop(cpu);
	new_PC |= ((Word) stack_pop(cpu)) << 8;
	cpu->PC = new_PC;
	return 0;
}

Byte RTS(CPU *cpu){
	Word new_PC = (Word) stack_pop(cpu);
	new_PC |= ((Word) stack_pop(cpu)) << 8;
	cpu->PC = new_PC + 1;
	return 0;
}

Byte SBC(CPU *cpu){
	Word acc = (Word) cpu->A;
	Word tmp_byte = ((Word) cpu->temp_byte) ^ 0x00FF;
	Word temp_sum = acc + tmp_byte + cpu->C;

	cpu->A = (Byte) temp_sum;
	cpu->C = (temp_sum > 256) ? 1 : 0;
	cpu->Z = (temp_sum == 0) ? 1 : 0;
	cpu->V = (~(acc ^ tmp_byte) & (acc ^ temp_sum) & 0x80) ? 1 : 0;
	cpu->N = (temp_sum & 0x80) ? 1 : 0;
	return 0;
}

Byte SEC(CPU *cpu){
	cpu->C = 1;
	return 0;
}

Byte SED(CPU *cpu){
	cpu->D = 1;
	return 0;
}

Byte SEI(CPU *cpu){
	cpu->I = 1;
	return 0;
}

Byte STA(CPU *cpu){
	write_byte(cpu, cpu->temp_word, cpu->A);
	return 0;
}

Byte STX(CPU *cpu){
	write_byte(cpu, cpu->temp_word, cpu->X);
	return 0;
}

Byte STY(CPU *cpu){
	write_byte(cpu, cpu->temp_word, cpu->Y);
	return 0;
}

Byte TAX(CPU *cpu){
	cpu->X = cpu->A;
	cpu->Z = (cpu->X == 0) ? 1 : 0;
	cpu->N = (cpu->X & 0x80) ? 1 : 0;
	return 0;
}

Byte TAY(CPU *cpu){
	cpu->Y = cpu->A;
	cpu->Z = (cpu->Y == 0) ? 1 : 0;
	cpu->N = (cpu->Y & 0x80) ? 1 : 0;
	return 0;
}

Byte TSX(CPU *cpu){
	cpu->X = (Byte) cpu->SP;
	cpu->Z = (cpu->X == 0) ? 1 : 0;
	cpu->N = (cpu->X & 0x80) ? 1 : 0;
	return 0;
}

Byte TXA(CPU *cpu){
	cpu->X = cpu->A;
	cpu->Z = (cpu->X == 0) ? 1 : 0;
	cpu->N = (cpu->X & 0x80) ? 1 : 0;
	return 0;
}

Byte TXS(CPU *cpu){
	cpu->SP = (Word) cpu->X;
	cpu->SP |= 0x100;
	return 0;
}

Byte TYA(CPU *cpu){
	cpu->Y = cpu->A;
	cpu->Z = (cpu->Y == 0) ? 1 : 0;
	cpu->N = (cpu->Y & 0x80) ? 1 : 0;
	return 0;
}

Byte NULL_OP(CPU *cpu){
	return 0;
}


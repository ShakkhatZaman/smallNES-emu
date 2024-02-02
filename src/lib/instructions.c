#include <stdint.h>
#include "6502.h"
#include "instructions.h"
#include "ppu.h"

const Ins ins_table[256] = {
	{IMP, BRK, 7, 7}, {INX, ORA, 6, 6}, NULL_INS, NULL_INS, NULL_INS, {ZP0, ORA, 3, 3}, {ZP0, ASL, 5, 5}, NULL_INS, {IMP, PHP, 3, 3}, {IMM, ORA, 2, 2}, {ACC, ASL, 2, 2}, NULL_INS, NULL_INS, {ABS, ORA, 4, 4}, {ABS, ASL, 6, 6}, NULL_INS,
	{REL, BPL, 2, 4}, {INY, ORA, 5, 6}, NULL_INS, NULL_INS, NULL_INS, {ZPX, ORA, 4, 4}, {ZPX, ASL, 6, 6}, NULL_INS, {IMP, CLC, 2, 2}, {ABX, ORA, 4, 5}, NULL_INS, NULL_INS, NULL_INS, {ABX, ORA, 4, 5}, {ABX, ASL, 7, 7}, NULL_INS,
	{ABS, JSR, 6, 6}, {INX, AND, 6, 6}, NULL_INS, NULL_INS, {ZP0, BIT, 3, 3}, {ZP0, AND, 3, 3}, {ZP0, ROL, 5, 5}, NULL_INS, {IMP, PLP, 4, 4}, {IMM, AND, 2, 2}, {ACC, ROL, 2, 2}, NULL_INS, {ABS, BIT, 4, 4}, {ABS, AND, 4, 4}, {ABS, ROL, 6, 6}, NULL_INS,
	{REL, BMI, 2, 4}, {INY, AND, 6, 6}, NULL_INS, NULL_INS, NULL_INS, {ZPX, AND, 4, 4}, {ZPX, ROL, 6, 6}, NULL_INS, {IMP, SEC, 2, 2}, {ABY, AND, 4, 5}, NULL_INS, NULL_INS, NULL_INS, {ABX, AND, 4, 5}, {ABX, ROL, 7, 7}, NULL_INS,
	{IMP, RTI, 6, 6}, {INX, EOR, 6, 6}, NULL_INS, NULL_INS, NULL_INS, {ZP0, EOR, 3, 3}, {ZP0, LSR, 5, 5}, NULL_INS, {IMP, PHA, 3, 3}, {IMM, EOR, 2, 2}, {ACC, LSR, 2, 2}, NULL_INS, {ABS, JMP, 3, 3}, {ABS, EOR, 4, 4}, {ABS, LSR, 6, 6}, NULL_INS,
	{REL, BVC, 2, 4}, {INY, EOR, 5, 6}, NULL_INS, NULL_INS, NULL_INS, {ZPX, EOR, 4, 4}, {ZPX, LSR, 6, 6}, NULL_INS, {IMP, CLI, 2, 2}, {ABY, EOR, 4, 5}, NULL_INS, NULL_INS, NULL_INS, {ABX, EOR, 4, 5}, {ABX, LSR, 7, 7}, NULL_INS,
	{IMP, RTS, 6, 6}, {INX, ADC, 6, 6}, NULL_INS, NULL_INS, NULL_INS, {ZP0, ADC, 3, 3}, {ZP0, ROR, 5, 5}, NULL_INS, {IMP, PLA, 4, 4}, {IMM, ADC, 2, 2}, {ACC, ROR, 2, 2}, NULL_INS, {IND, JMP, 5, 5}, {ABS, ADC, 4, 4}, {ABS, ROR, 6, 6}, NULL_INS,
	{REL, BVS, 2, 4}, {INY, ADC, 5, 6}, NULL_INS, NULL_INS, NULL_INS, {ZPX, ADC, 4, 4}, {ZPX, ROR, 6, 6}, NULL_INS, {IMP, SEI, 2, 2}, {ABY, ADC, 4, 5}, NULL_INS, NULL_INS, NULL_INS, {ABX, ADC, 4, 5}, {ABX, ROR, 7, 7}, NULL_INS,
	NULL_INS, {INX, STA, 6, 6}, NULL_INS, NULL_INS, {ZP0, STY, 3, 3}
};

/*Helper functions */

Byte fetch_byte(CPU *cpu){
	Word counter = cpu->PC;
	cpu->PC = (counter == 0xFFFF) ? 0x8000 : counter + 1;
	Byte data = 0x00;
	//printf("%x\n", cpu->PC);
	if (counter >= 0x4020 && counter <= 0xFFFF)
		data = cpu->p_Bus->mapper->cpu_read(cpu->p_Bus->mapper, counter);
	return data;
}

Word fetch_word(CPU *cpu){
	Word counter = cpu->PC;
	Word data = 0x0000;
	cpu->PC = (counter == 0xFFFF) ? 0x8001 : counter + 2;
	if (counter >= 0x4020 && counter <= 0xFFFF){
		data = (Word) cpu->p_Bus->mapper->cpu_read(cpu->p_Bus->mapper, counter);
		data |= ((Word) cpu->p_Bus->mapper->cpu_read(cpu->p_Bus->mapper, counter + 1)) << 8;
	}
	return data;
}

Byte cpu_read_byte(CPU *cpu, Word address){
	Byte data = 0x00;
	if (address >= 0x0000 && address <= 0x1FFF) 		// Address inside CPU RAM 
		data = cpu->p_Bus->RAM[address & 0x07FF];

	else if (address >= 0x2000 && address <= 0x3FFF)	// Address inside PPU registers
		data = cpu_to_ppu_read(cpu->p_ppu, address);
	
	else
		data = cpu->p_Bus->mapper->cpu_read(cpu->p_Bus->mapper, address); // Address inside cartridge

	return data;
}

Byte cpu_write_byte(CPU *cpu, Word address, Byte data){
	if (address >= 0x0000 && address <= 0x1FFF)			// Address inside CPU RAM 
		cpu->p_Bus->RAM[address & 0x07FF] = data;

	else if (address >= 0x2000 && address <= 0x3FFF)	// Address inside PPU registers
		cpu_to_ppu_write(cpu->p_ppu, address, data);

	else
		cpu->p_Bus->mapper->cpu_write(cpu->p_Bus->mapper, address, data);

	return 0;
}

void stack_push(CPU *cpu, Byte data){
	cpu->p_Bus->RAM[cpu->SP] = data;
	cpu->SP = (cpu->SP == 0x0100) ? 0x01FF : cpu->SP - 1;
}

Byte stack_pop(CPU *cpu){
	cpu->SP = (cpu->SP == 0x01FF) ? 0x0100 : cpu->SP + 1;
	return cpu->p_Bus->RAM[cpu->SP];
}

void set_status_A(CPU *cpu){
	cpu->Z = (cpu->A == 0); // Zero flag is set if A is Zero
	cpu->N = (cpu->A & 0b10000000) ? 1 : 0;

}

/*Addressing modes*/

Byte ABS(CPU *cpu){
	Word data_address = fetch_word(cpu);
	cpu->temp_word = data_address;
	cpu->temp_byte = cpu_read_byte(cpu, data_address);
	cpu->current_mode = 1;
	return 0;
}

Byte ABX(CPU *cpu){
	Word data_address = fetch_word(cpu);
	Word data_address_x = data_address + cpu->X;
	cpu->temp_word = data_address_x;
	cpu->temp_byte = cpu_read_byte(cpu, data_address_x);
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
	cpu->temp_byte = cpu_read_byte(cpu, data_address_y);
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
	Byte byte_data = cpu_read_byte(cpu, data_address);
	Word word_data;

	if ((data_address & 0x00FF) == 0x00FF)
		word_data = (((Word) cpu_read_byte(cpu, data_address & 0xFF00)) << 8) | ((Word) byte_data);

	else 
		word_data = (((Word) cpu_read_byte(cpu, data_address + 1)) << 8) | ((Word) byte_data);

	cpu->temp_word = word_data;
	cpu->temp_byte = cpu_read_byte(cpu, word_data);
	cpu->current_mode = 7;
	return 0;
}

Byte IZX(CPU *cpu){
	Byte zp_data_address_x = fetch_byte(cpu);
	zp_data_address_x += cpu->X;
	Word full_zp_address_x = (Word) zp_data_address_x;
	Byte byte_data = cpu_read_byte(cpu, full_zp_address_x);
	Word word_data = (((Word) cpu_read_byte(cpu, full_zp_address_x + 1)) << 8) | ((Word) byte_data);
	cpu->temp_word = word_data;
	cpu->temp_byte = cpu_read_byte(cpu, word_data);
	cpu->current_mode = 8;
	return 0;
}

Byte IZY(CPU *cpu){
	Byte zp_data_address_y = fetch_byte(cpu);
	Word full_zp_address_y = (Word) zp_data_address_y;
	Byte byte_data = cpu_read_byte(cpu, full_zp_address_y);
	Word word_data = (((Word) cpu_read_byte(cpu, full_zp_address_y + 1)) << 8) | ((Word) byte_data);
	Word temp_word_data = (Word) cpu->Y + word_data;
	cpu->temp_word = word_data;
	cpu->temp_byte = cpu_read_byte(cpu, word_data);
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
	cpu->temp_byte = cpu_read_byte(cpu, (Word) zero_page_address);
	cpu->current_mode = 11;
	return 0;
}

Byte ZPX(CPU *cpu){
	Byte zero_page_address_x = fetch_byte(cpu);
	zero_page_address_x += cpu->X;
	cpu->temp_word = (Word) zero_page_address_x;
	cpu->temp_byte = cpu_read_byte(cpu, (Word) zero_page_address_x);
	cpu->current_mode = 12;
	return 0;
}

Byte ZPY(CPU *cpu){
	Byte zero_page_address_y = fetch_byte(cpu);
	zero_page_address_y += cpu->Y;
	cpu->temp_word = (Word) zero_page_address_y;
	cpu->temp_byte = cpu_read_byte(cpu, (Word) zero_page_address_y);
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
		cpu_write_byte(cpu, cpu->temp_word, cpu->temp_byte);
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
	cpu_write_byte(cpu, cpu->temp_word, cpu->temp_byte);
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
	cpu_write_byte(cpu, cpu->temp_word, cpu->temp_byte);
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
		cpu_write_byte(cpu, cpu->temp_word, cpu->temp_byte);
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
		cpu_write_byte(cpu, cpu->temp_word, cpu->temp_byte);
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
		cpu_write_byte(cpu, cpu->temp_word, cpu->temp_byte);
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
	cpu_write_byte(cpu, cpu->temp_word, cpu->A);
	return 0;
}

Byte STX(CPU *cpu){
	cpu_write_byte(cpu, cpu->temp_word, cpu->X);
	return 0;
}

Byte STY(CPU *cpu){
	cpu_write_byte(cpu, cpu->temp_word, cpu->Y);
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


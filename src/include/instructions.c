#include "instructions.h"
#include "6502.h"
#include "ppu.h"

/*Helper functions */

static Byte cpu_read_byte(CPU *cpu, Word address);
static Byte cpu_write_byte(CPU *cpu, Word address, Byte data);
static void stack_push(CPU *cpu, Byte data);
static Byte stack_pop(CPU *cpu);
static void _set_status_A(CPU *cpu);
static void _check_page_crossed(CPU *cpu);

Byte fetch_byte(CPU *cpu) {
    Word counter = cpu->PC;
    cpu->PC = (counter == 0xFFFF) ? 0x8000 : counter + 1;
    Byte data = 0x00;
    if (counter >= 0x4020)
        data = cpu->p_Bus->mapper->cpu_read(cpu->p_Bus->mapper, counter);
    return data;
}

Word fetch_word(CPU *cpu) {
    Word counter = cpu->PC;
    Word data = 0x0000;
    cpu->PC = (counter == 0xFFFF) ? 0x8001 : counter + 2;
    if (counter >= 0x4020) {
        data = (Word) cpu->p_Bus->mapper->cpu_read(cpu->p_Bus->mapper, counter);
        cpu_clock(cpu);
        data |= ((Word) cpu->p_Bus->mapper->cpu_read(cpu->p_Bus->mapper, counter + 1)) << 8;
    }
    return data;
}

static Byte cpu_read_byte(CPU *cpu, Word address) {
    Byte data = 0x00;
    // Address inside CPU RAM
    if (address <= 0x1FFF)
        data = cpu->p_Bus->RAM[address & 0x07FF];		// 2KB of RAM mirrored across 8KB

        // Address inside PPU registers
    else if (address >= 0x2000 && address <= 0x3FFF)
        data = cpu_to_ppu_read(cpu->p_ppu, address);	// Reading on the ppu registers

        // Address inside cartridge
    else
        data = cpu->p_Bus->mapper->cpu_read(cpu->p_Bus->mapper, address);

    return data;
}

static Byte cpu_write_byte(CPU *cpu, Word address, Byte data) {
    // Address inside CPU RAM
    if (address <= 0x1FFF)
        cpu->p_Bus->RAM[address & 0x07FF] = data; 		// 2KB of RAM mirrored across 8KB

        // Address inside PPU registers
    else if (address >= 0x2000 && address <= 0x3FFF)
        cpu_to_ppu_write(cpu->p_ppu, address, data);	// Writting on the ppu registers

        // Address inside cartridge
    else
        cpu->p_Bus->mapper->cpu_write(cpu->p_Bus->mapper, address, data);

    return 0;
}

static void stack_push(CPU *cpu, Byte data) {
    cpu->p_Bus->RAM[cpu->SP] = data;
    cpu->SP = (cpu->SP == 0x0100) ? 0x01FF : cpu->SP - 1;
}

static Byte stack_pop(CPU *cpu) {
    cpu->SP = (cpu->SP == 0x01FF) ? 0x0100 : cpu->SP + 1;
    return cpu->p_Bus->RAM[cpu->SP];
}

static void _set_status_A(CPU *cpu) {
    cpu->Z = (cpu->A == 0); // Zero flag is set if A is Zero
    cpu->N = (cpu->A & 0b10000000) ? 1 : 0;
}

static void _check_page_crossed(CPU *cpu) {
    if ((cpu->PC & 0xFF00) == (cpu->temp_word & 0xFF00))
        cpu_clock(cpu);
    else {
        cpu_clock(cpu);
        cpu_clock(cpu);
    }
    cpu->PC = cpu->temp_word;
}

void cpu_irq(CPU *cpu) {
    if (!(cpu->I)) {
        cpu->PC += 1;
        cpu_clock(cpu);
        stack_push(cpu, (Byte) (cpu->PC >> 8));
        cpu_clock(cpu);
        stack_push(cpu, (Byte) cpu->PC);
        cpu_clock(cpu);
        Byte SR = 0;
        SR |= (cpu->N) ? 0x80 : 0;
        SR |= (cpu->V) ? 0x40 : 0;
        SR |= (cpu->D) ? 0x8 : 0;
        SR |= (cpu->I) ? 0x4 : 0;
        SR |= (cpu->Z) ? 0x2 : 0;
        SR |= (cpu->C) ? 0x1 : 0;
        stack_push(cpu, SR);
        cpu_clock(cpu);
        cpu->I = 1;
        Word new_address = cpu_read_byte(cpu, 0xFFFE);
        cpu_clock(cpu);
        new_address |= ((Word) cpu_read_byte(cpu, 0xFFFF)) << 8;
        cpu->PC = new_address;
        cpu_clock(cpu);
    }
}

void cpu_nmi(CPU *cpu) {
    cpu->PC += 1;
    cpu_clock(cpu);
    stack_push(cpu, (Byte) (cpu->PC >> 8));
    cpu_clock(cpu);
    stack_push(cpu, (Byte) cpu->PC);
    cpu_clock(cpu);
    Byte SR = 0;
    SR |= (cpu->N) ? 0x80 : 0;
    SR |= (cpu->V) ? 0x40 : 0;
    SR |= (cpu->D) ? 0x8 : 0;
    SR |= (cpu->I) ? 0x4 : 0;
    SR |= (cpu->Z) ? 0x2 : 0;
    SR |= (cpu->C) ? 0x1 : 0;
    stack_push(cpu, SR);
    cpu_clock(cpu);
    cpu->I = 1;
    Word new_address = cpu_read_byte(cpu, 0xFFFA);
    cpu_clock(cpu);
    new_address |= ((Word) cpu_read_byte(cpu, 0xFFFB)) << 8;
    cpu->PC = new_address;
    cpu_clock(cpu);
}

const Ins ins_table[256] = {
    /*\--- 0 ---\ \--- 1 ---\ \--- 2 ---\ \--- 3 ---\ \--- 4 ---\ \--- 5 ---\ \--- 6 ---\ \--- 7 ---\ \--- 8 ---\ \--- 9 ---\ \--- A ---\ \--- B ---\ \--- C ---\ \--- D ---\ \--- E ---\ \--- F ---\*/
    /*= 0 =*/	{IMP, BRK}, {IZX, ORA}, {NUL, NUL}, {NUL, NUL}, {NUL, NUL}, {ZP0, ORA}, {ZP0, ASL}, {NUL, NUL}, {IMP, PHP}, {IMM, ORA}, {ACC, ASL}, {NUL, NUL}, {NUL, NUL}, {ABS, ORA}, {ABS, ASL}, {NUL, NUL},
    /*= 1 =*/	{REL, BPL}, {IZY, ORA}, {NUL, NUL}, {NUL, NUL}, {NUL, NUL}, {ZPX, ORA}, {ZPX, ASL}, {NUL, NUL}, {IMP, CLC}, {ABX, ORA}, {NUL, NUL}, {NUL, NUL}, {NUL, NUL}, {ABX, ORA}, {ABX, ASL}, {NUL, NUL},
    /*= 2 =*/	{ABS, JSR}, {IZX, AND}, {NUL, NUL}, {NUL, NUL}, {ZP0, BIT}, {ZP0, AND}, {ZP0, ROL}, {NUL, NUL}, {IMP, PLP}, {IMM, AND}, {ACC, ROL}, {NUL, NUL}, {ABS, BIT}, {ABS, AND}, {ABS, ROL}, {NUL, NUL},
    /*= 3 =*/	{REL, BMI}, {IZY, AND}, {NUL, NUL}, {NUL, NUL}, {NUL, NUL}, {ZPX, AND}, {ZPX, ROL}, {NUL, NUL}, {IMP, SEC}, {ABY, AND}, {NUL, NUL}, {NUL, NUL}, {NUL, NUL}, {ABX, AND}, {ABX, ROL}, {NUL, NUL},
    /*= 4 =*/	{IMP, RTI}, {IZX, EOR}, {NUL, NUL}, {NUL, NUL}, {NUL, NUL}, {ZP0, EOR}, {ZP0, LSR}, {NUL, NUL}, {IMP, PHA}, {IMM, EOR}, {ACC, LSR}, {NUL, NUL}, {ABS, JMP}, {ABS, EOR}, {ABS, LSR}, {NUL, NUL},
    /*= 5 =*/	{REL, BVC}, {IZY, EOR}, {NUL, NUL}, {NUL, NUL}, {NUL, NUL}, {ZPX, EOR}, {ZPX, LSR}, {NUL, NUL}, {IMP, CLI}, {ABY, EOR}, {NUL, NUL}, {NUL, NUL}, {NUL, NUL}, {ABX, EOR}, {ABX, LSR}, {NUL, NUL},
    /*= 6 =*/	{IMP, RTS}, {IZX, ADC}, {NUL, NUL}, {NUL, NUL}, {NUL, NUL}, {ZP0, ADC}, {ZP0, ROR}, {NUL, NUL}, {IMP, PLA}, {IMM, ADC}, {ACC, ROR}, {NUL, NUL}, {IND, JMP}, {ABS, ADC}, {ABS, ROR}, {NUL, NUL},
    /*= 7 =*/	{REL, BVS}, {IZY, ADC}, {NUL, NUL}, {NUL, NUL}, {NUL, NUL}, {ZPX, ADC}, {ZPX, ROR}, {NUL, NUL}, {IMP, SEI}, {ABY, ADC}, {NUL, NUL}, {NUL, NUL}, {NUL, NUL}, {ABX, ADC}, {ABX, ROR}, {NUL, NUL},
    /*= 8 =*/	{NUL, NUL}, {IZX, STA}, {NUL, NUL}, {NUL, NUL}, {ZP0, STY}, {ZP0, STA}, {ZP0, STX}, {NUL, NUL}, {IMP, DEY}, {NUL, NUL}, {IMP, TXA}, {NUL, NUL}, {ABS, STY}, {ABS, STA}, {ABS, STX}, {NUL, NUL},
    /*= 9 =*/	{REL, BCC}, {IZY, STA}, {NUL, NUL}, {NUL, NUL}, {ZPX, STY}, {ZPX, STA}, {ZPY, STX}, {NUL, NUL}, {IMP, TYA}, {ABY, STA}, {IMP, TXS}, {NUL, NUL}, {NUL, NUL}, {ABX, STA}, {NUL, NUL}, {NUL, NUL},
    /*= A =*/	{IMM, LDY}, {IZX, LDA}, {IMM, LDX}, {NUL, NUL}, {ZP0, LDY}, {ZP0, LDA}, {ZP0, LDX}, {NUL, NUL}, {IMP, TAY}, {IMM, LDA}, {IMP, TAX}, {NUL, NUL}, {ABS, LDY}, {ABS, LDA}, {ABS, LDX}, {NUL, NUL},
    /*= B =*/	{REL, BCS}, {IZY, LDA}, {NUL, NUL}, {NUL, NUL}, {ZPX, LDY}, {ZPX, LDA}, {ZPY, LDX}, {NUL, NUL}, {IMP, CLV}, {ABY, LDA}, {IMP, TSX}, {NUL, NUL}, {ABX, LDY}, {ABX, LDA}, {ABY, LDX}, {NUL, NUL},
    /*= C =*/	{IMM, CPY}, {IZX, CMP}, {NUL, NUL}, {NUL, NUL}, {ZP0, CPY}, {ZP0, CMP}, {ZP0, DEC}, {NUL, NUL}, {IMP, INY}, {IMM, CMP}, {IMP, DEX}, {NUL, NUL}, {ABS, CPY}, {ABS, CMP}, {ABS, DEC}, {NUL, NUL},
    /*= D =*/	{REL, BNE}, {IZY, CMP}, {NUL, NUL}, {NUL, NUL}, {NUL, NUL}, {ZPX, CMP}, {ZPX, DEC}, {NUL, NUL}, {IMP, CLD}, {ABY, CMP}, {NUL, NUL}, {NUL, NUL}, {NUL, NUL}, {ABX, CMP}, {ABX, DEC}, {NUL, NUL},
    /*= E =*/	{IMM, CPX}, {IZX, SBC}, {NUL, NUL}, {NUL, NUL}, {ZP0, CPX}, {ZP0, SBC}, {ZP0, INC}, {NUL, NUL}, {IMP, INX}, {IMM, SBC}, {IMP, NOP}, {NUL, NUL}, {ABS, CPX}, {ABS, SBC}, {ABS, INC}, {NUL, NUL},
    /*= F =*/	{REL, BEQ}, {IZY, SBC}, {NUL, NUL}, {NUL, NUL}, {NUL, NUL}, {ZPX, SBC}, {ZPX, INC}, {NUL, NUL}, {IMP, SED}, {ABY, SBC}, {NUL, NUL}, {NUL, NUL}, {NUL, NUL}, {ABX, SBC}, {ABX, INC}, {NUL, NUL}
};

/*Addressing modes*/

Byte ABS(CPU *cpu) {
    Word data_address = fetch_word(cpu);
    cpu->temp_word = data_address;
    cpu->current_mode = 1;
    return 0;
}

Byte ABX(CPU *cpu) {
    Word data_address = fetch_word(cpu);
    cpu_clock(cpu);
    Word data_address_x = data_address + cpu->X;
    cpu->temp_word = data_address_x;
    cpu->current_mode = 2;
    if ((data_address & 0xFF00) == (data_address_x & 0xFF00))
        return 0;
    else{
        cpu_clock(cpu);
        return 1;
    }
}

Byte ABY(CPU *cpu) {
    Word data_address = fetch_word(cpu);
    cpu_clock(cpu);
    Word data_address_y = data_address + cpu->Y;
    cpu->temp_word = data_address_y;
    cpu->current_mode = 3;
    if ((data_address & 0xFF00) == (data_address_y & 0xFF00))
        return 0;
    else{
        cpu_clock(cpu);
        return 1;
    }
}

Byte ACC(CPU *cpu) {
    cpu->temp_byte = cpu->A;
    cpu->current_mode = 4;
    return 0;
}

Byte IMP(CPU *cpu) {
    cpu->current_mode = 5;
    return 0;
}

Byte IMM(CPU *cpu) {
    cpu->temp_byte = fetch_byte(cpu);
    cpu->current_mode = 6;
    return 0;
}

Byte IND(CPU *cpu) {
    Word data_address = fetch_word(cpu);
    Byte byte_data = cpu_read_byte(cpu, data_address);
    cpu_clock(cpu);
    Word word_data = 0;

    if ((data_address & 0x00FF) == 0x00FF) {
        word_data = (((Word) cpu_read_byte(cpu, data_address & 0xFF00)) << 8) | ((Word) byte_data);
        cpu_clock(cpu);
    }
    else {
        word_data = (((Word) cpu_read_byte(cpu, data_address + 1)) << 8) | ((Word) byte_data);
        cpu_clock(cpu);
    }
    cpu->temp_word = word_data;
    cpu->temp_byte = cpu_read_byte(cpu, word_data);
    cpu->current_mode = 7;
    return 0;
}

Byte IZX(CPU *cpu) {
    Byte zp_data_address_x = fetch_byte(cpu);
    cpu_clock(cpu);
    zp_data_address_x += cpu->X;
    Word full_zp_address_x = (Word) zp_data_address_x;
    cpu_clock(cpu);
    Byte byte_data = cpu_read_byte(cpu, full_zp_address_x);
    cpu_clock(cpu);
    Word word_data = (((Word) cpu_read_byte(cpu, full_zp_address_x + 1)) << 8) | ((Word) byte_data);
    cpu_clock(cpu);
    cpu->temp_word = word_data;
    cpu->temp_byte = cpu_read_byte(cpu, word_data);
    cpu->current_mode = 8;
    return 0;
}

Byte IZY(CPU *cpu) {
    Byte zp_data_address_y = fetch_byte(cpu);
    cpu_clock(cpu);
    Word full_zp_address_y = (Word) zp_data_address_y;
    Byte byte_data = cpu_read_byte(cpu, full_zp_address_y);
    cpu_clock(cpu);
    Word word_data = (((Word) cpu_read_byte(cpu, full_zp_address_y + 1)) << 8) | ((Word) byte_data);
    cpu_clock(cpu);
    Word temp_word_data = (Word) cpu->Y + word_data;
    cpu->temp_word = temp_word_data;
    cpu->temp_byte = cpu_read_byte(cpu, temp_word_data);
    cpu->current_mode = 9;
    if ((temp_word_data & 0xFF00) == (word_data & 0xFF00))
        return 0;
    else {
        cpu_clock(cpu);
        return 1;
    }
}

Byte REL(CPU *cpu) {
    Byte byte_offset = fetch_byte(cpu);
    cpu->temp_byte = byte_offset;
    Word target = cpu->PC;
    target += (int8_t) byte_offset;

    cpu->temp_word = target;
    cpu->current_mode = 10;
    return 0;
}

Byte ZP0(CPU *cpu) {
    Byte zero_page_address = fetch_byte(cpu);
    cpu_clock(cpu);
    cpu->temp_word = (Word) zero_page_address;
    cpu->temp_byte = cpu_read_byte(cpu, (Word) zero_page_address);
    cpu->current_mode = 11;
    return 0;
}

Byte ZPX(CPU *cpu) {
    Byte zero_page_address_x = fetch_byte(cpu);
    cpu_clock(cpu);
    zero_page_address_x += cpu->X;
    cpu->temp_word = (Word) zero_page_address_x;
    cpu_clock(cpu);
    cpu->temp_byte = cpu_read_byte(cpu, (Word) zero_page_address_x);
    cpu->current_mode = 12;
    return 0;
}

Byte ZPY(CPU *cpu) {
    Byte zero_page_address_y = fetch_byte(cpu);
    cpu_clock(cpu);
    zero_page_address_y += cpu->Y;
    cpu->temp_word = (Word) zero_page_address_y;
    cpu_clock(cpu);
    cpu->temp_byte = cpu_read_byte(cpu, (Word) zero_page_address_y);
    cpu->current_mode = 13;
    return 0;
}

/*Operation functions*/

Byte ADC(CPU *cpu) {
    if (cpu->current_mode == 1 || cpu->current_mode == 2 || cpu->current_mode == 3) cpu->temp_byte = cpu_read_byte(cpu, cpu->temp_word);
    Word acc = (Word) cpu->A;
    Word tmp_byte = (Word) cpu->temp_byte;
    Word temp_sum = acc + tmp_byte + cpu->C;

    cpu->A = (Byte) temp_sum;
    if (cpu->current_mode == 1)
        cpu_clock(cpu);
    cpu->C = (temp_sum > 256) ? 1 : 0;
    cpu->Z = (temp_sum == 0) ? 1 : 0;
    cpu->V = (~(acc ^ tmp_byte) & (acc ^ temp_sum) & 0x80) ? 1 : 0;
    cpu->N = (temp_sum & 0x80) ? 1 : 0;
    cpu_clock(cpu);
    return 0;
}

Byte AND(CPU *cpu){
    if (cpu->current_mode == 1 || cpu->current_mode == 2 || cpu->current_mode == 3) cpu->temp_byte = cpu_read_byte(cpu, cpu->temp_word);
    if (cpu->current_mode == 1)
        cpu_clock(cpu);
    cpu->A &= cpu->temp_byte;
    cpu->Z = (cpu->A == 0) ? 1 : 0;
    cpu->N = (cpu->A & 0x80) ? 1 : 0;
    cpu_clock(cpu);
    return 0;
}

Byte ASL(CPU *cpu){
    if (cpu->current_mode == 4){
        cpu->C = (cpu->A & 0x80) ? 1 : 0;
        cpu->A <<= 1;
        cpu->Z = (cpu->A == 0) ? 1 : 0;
        cpu->N = (cpu->A & 0x80) ? 1 : 0;
    }
    else {
        if (cpu->current_mode == 1 || cpu->current_mode == 2 || cpu->current_mode == 3) cpu->temp_byte = cpu_read_byte(cpu, cpu->temp_word);
        cpu->C = (cpu->temp_byte & 0x80) ? 1 : 0;
        cpu_clock(cpu);
        cpu->temp_byte >>= 1;
        cpu_clock(cpu);
        cpu->Z = (cpu->temp_byte == 0) ? 1 : 0;
        cpu->N = (cpu->temp_byte & 0x80) ? 1 : 0;
        if (cpu->current_mode == 1 || cpu->current_mode == 2)
            cpu_clock(cpu);
        cpu_write_byte(cpu, cpu->temp_word, cpu->temp_byte);
    }
    cpu_clock(cpu);
    return 0;
}

Byte BCC(CPU *cpu){
    if (!cpu->C)
        _check_page_crossed(cpu);
    cpu_clock(cpu);
    return 0;
}

Byte BCS(CPU *cpu){
    if (cpu->C)
        _check_page_crossed(cpu);
    cpu_clock(cpu);
    return 0;
}

Byte BEQ(CPU *cpu){
    if (cpu->Z)
        _check_page_crossed(cpu);

    cpu_clock(cpu);
    return 0;
}

Byte BIT(CPU *cpu){
    if (cpu->current_mode == 1)
        cpu_clock(cpu);
    if (cpu->current_mode == 1 || cpu->current_mode == 2 || cpu->current_mode == 3) cpu->temp_byte = cpu_read_byte(cpu, cpu->temp_word);
    cpu->N = (cpu->temp_byte & 0b10000000) ? 1 : 0;
    cpu->V = (cpu->temp_byte & 0b01000000) ? 1 : 0;
    cpu->Z = (cpu->temp_byte & cpu->A) ? 0 : 1;
    cpu_clock(cpu);
    return 0;
}

Byte BMI(CPU *cpu){
    if (cpu->N)
        _check_page_crossed(cpu);
    cpu_clock(cpu);
    return 0;
}

Byte BNE(CPU *cpu){
    if (!cpu->Z)
        _check_page_crossed(cpu);
    cpu_clock(cpu);
    return 0;
}

Byte BPL(CPU *cpu){
    if (!cpu->N)
        cpu->PC = cpu->temp_word;
    cpu_clock(cpu);
    return 0;
}

Byte BRK(CPU *cpu){
    cpu->PC += 1;
    stack_push(cpu, (Byte) (cpu->PC >> 8));
    cpu_clock(cpu);
    stack_push(cpu, (Byte) (cpu->PC));
    cpu_clock(cpu);
    Byte SR = 0;
    SR |= (cpu->N) ? 0x80 : 0;
    SR |= (cpu->V) ? 0x40 : 0;
    SR |= 0x10;
    SR |= (cpu->D) ? 0x8 : 0;
    SR |= (cpu->I) ? 0x4 : 0;
    SR |= (cpu->Z) ? 0x2 : 0;
    SR |= (cpu->C) ? 0x1 : 0;
    stack_push(cpu, SR);
    cpu_clock(cpu);
    cpu->I = 1;
    cpu_clock(cpu);
    Word new_address = cpu_read_byte(cpu, 0xFFFE);
    cpu_clock(cpu);
    new_address |= ((Word) cpu_read_byte(cpu, 0xFFFF)) << 8;
    cpu->PC = new_address;
    cpu_clock(cpu);
    return 0;
}

Byte BVC(CPU *cpu){
    if (!cpu->V)
        _check_page_crossed(cpu);
    cpu_clock(cpu);
    return 0;
}

Byte BVS(CPU *cpu){
    if (cpu->V)
        _check_page_crossed(cpu);
    cpu_clock(cpu);
    return 0;
}

Byte CLC(CPU *cpu){
    cpu->C = 0;
    cpu_clock(cpu);
    return 0;
}

Byte CLD(CPU *cpu){
    cpu->D = 0;
    cpu_clock(cpu);
    return 0;
}

Byte CLI(CPU *cpu){
    cpu->I = 0;
    cpu_clock(cpu);
    return 0;
}

Byte CLV(CPU *cpu){
    cpu->V = 0;
    cpu_clock(cpu);
    return 0;
}

Byte CMP(CPU *cpu){
    if (cpu->current_mode == 1 || cpu->current_mode == 2 || cpu->current_mode == 3) cpu->temp_byte = cpu_read_byte(cpu, cpu->temp_word);
    Word a = (Word) cpu->A;
    Word mem = (Word) cpu->temp_byte;
    Word result = a - mem;
    if (cpu->current_mode == 1)
        cpu_clock(cpu);
    cpu->C = (a >= mem) ? 1 : 0;
    cpu->Z = ((result & 0xFF) == 0) ? 1 : 0;
    cpu->N = (result & 0x80) ? 1 : 0;
    cpu_clock(cpu);
    return 0;
}

Byte CPX(CPU *cpu){
    if (cpu->current_mode == 1 || cpu->current_mode == 2 || cpu->current_mode == 3) cpu->temp_byte = cpu_read_byte(cpu, cpu->temp_word);
    Word x = (Word) cpu->X;
    Word mem = (Word) cpu->temp_byte;
    Word result = x - mem;
    if (cpu->current_mode == 1)
        cpu_clock(cpu);
    cpu->C = (x >= mem) ? 1 : 0;
    cpu->Z = ((result & 0xFF) == 0) ? 1 : 0;
    cpu->N = (result & 0x80) ? 1 : 0;
    cpu_clock(cpu);
    return 0;
}

Byte CPY(CPU *cpu){
    if (cpu->current_mode == 1 || cpu->current_mode == 2 || cpu->current_mode == 3) cpu->temp_byte = cpu_read_byte(cpu, cpu->temp_word);
    Word y = (Word) cpu->Y;
    Word mem = (Word) cpu->temp_byte;
    Word result = y - mem;
    if (cpu->current_mode == 1)
        cpu_clock(cpu);
    cpu->C = (y >= mem) ? 1 : 0;
    cpu->Z = ((result & 0xFF) == 0) ? 1 : 0;
    cpu->N = (result & 0x80) ? 1 : 0;
    cpu_clock(cpu);
    return 0;
}

Byte DEC(CPU *cpu){
    if (cpu->current_mode == 1 || cpu->current_mode == 2 || cpu->current_mode == 3) cpu->temp_byte = cpu_read_byte(cpu, cpu->temp_word);
    cpu->temp_byte -= 1; 
    cpu_clock(cpu);
    cpu_write_byte(cpu, cpu->temp_word, cpu->temp_byte);
    cpu_clock(cpu);
    if (cpu->current_mode == 1 || cpu->current_mode == 2)
        cpu_clock(cpu);
    cpu->Z = (cpu->temp_byte == 0) ? 1 : 0;
    cpu->N = (cpu->temp_byte & 0x80) ? 1 : 0;
    cpu_clock(cpu);
    return 0;
}

Byte DEX(CPU *cpu){
    cpu->X -= 1; 
    cpu->Z = (cpu->X == 0) ? 1 : 0;
    cpu->N = (cpu->X & 0x80) ? 1 : 0;
    cpu_clock(cpu);
    return 0;
}

Byte DEY(CPU *cpu){
    cpu->Y -= 1; 
    cpu->Z = (cpu->Y == 0) ? 1 : 0;
    cpu->N = (cpu->Y & 0x80) ? 1 : 0;
    cpu_clock(cpu);
    return 0;
}

Byte EOR(CPU *cpu){
    if (cpu->current_mode == 1 || cpu->current_mode == 2 || cpu->current_mode == 3) cpu->temp_byte = cpu_read_byte(cpu, cpu->temp_word);
    cpu->A ^= cpu->temp_byte;
    if (cpu->current_mode == 1)
        cpu_clock(cpu);
    cpu->Z = (cpu->A == 0) ? 1 : 0;
    cpu->N = (cpu->A & 0x80) ? 1 : 0;
    cpu_clock(cpu);
    return 0;
}

Byte INC(CPU *cpu){
    if (cpu->current_mode == 1 || cpu->current_mode == 2 || cpu->current_mode == 3) cpu->temp_byte = cpu_read_byte(cpu, cpu->temp_word);
    cpu->temp_byte += 1; 
    cpu_clock(cpu);
    cpu_write_byte(cpu, cpu->temp_word, cpu->temp_byte);
    cpu_clock(cpu);
    if (cpu->current_mode == 1 || cpu->current_mode == 2)
        cpu_clock(cpu);
    cpu->Z = (cpu->temp_byte == 0) ? 1 : 0;
    cpu->N = (cpu->temp_byte & 0x80) ? 1 : 0;
    cpu_clock(cpu);
    return 0;
}

Byte INX(CPU *cpu){
    cpu->X += 1; 
    cpu->Z = (cpu->X == 0) ? 1 : 0;
    cpu->N = (cpu->X & 0x80) ? 1 : 0;
    cpu_clock(cpu);
    return 0;
}

Byte INY(CPU *cpu){
    cpu->Y += 1; 
    cpu->Z = (cpu->Y == 0) ? 1 : 0;
    cpu->N = (cpu->Y & 0x80) ? 1 : 0;
    cpu_clock(cpu);
    return 0;
}

Byte JMP(CPU *cpu){
    cpu->PC = cpu->temp_word;
    cpu_clock(cpu);
    return 0;
}

Byte JSR(CPU *cpu){
    Word current_pc = cpu->PC - 1;
    cpu_clock(cpu);
    stack_push(cpu, (Byte) (current_pc >> 8));
    cpu_clock(cpu);
    stack_push(cpu, (Byte) current_pc);
    cpu_clock(cpu);
    cpu->PC = cpu->temp_word;
    cpu_clock(cpu);
    return 0;
}

Byte LDA(CPU *cpu){
    if (cpu->current_mode == 1 || cpu->current_mode == 2 || cpu->current_mode == 3) cpu->temp_byte = cpu_read_byte(cpu, cpu->temp_word);
    cpu->A = cpu->temp_byte;
    if (cpu->current_mode == 1)
        cpu_clock(cpu);
    _set_status_A(cpu);
    cpu_clock(cpu);
    return 0;
}

Byte LDX(CPU *cpu){
    if (cpu->current_mode == 1 || cpu->current_mode == 2 || cpu->current_mode == 3) cpu->temp_byte = cpu_read_byte(cpu, cpu->temp_word);
    cpu->X = cpu->temp_byte;
    if (cpu->current_mode == 1)
        cpu_clock(cpu);
    cpu->Z = (cpu->X == 0) ? 1 : 0;
    cpu->N = (cpu->X & 0x80) ? 1 : 0;
    cpu_clock(cpu);
    return 0;
}

Byte LDY(CPU *cpu){
    if (cpu->current_mode == 1 || cpu->current_mode == 2 || cpu->current_mode == 3) cpu->temp_byte = cpu_read_byte(cpu, cpu->temp_word);
    cpu->Y = cpu->temp_byte;
    if (cpu->current_mode == 1)
        cpu_clock(cpu);
    cpu->Z = (cpu->Y == 0) ? 1 : 0;
    cpu->N = (cpu->Y & 0x80) ? 1 : 0;
    cpu_clock(cpu);
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
        if (cpu->current_mode == 1 || cpu->current_mode == 2 || cpu->current_mode == 3) cpu->temp_byte = cpu_read_byte(cpu, cpu->temp_word);
        cpu->C = (cpu->temp_byte & 1) ? 1 : 0;
        cpu_clock(cpu);
        cpu->temp_byte >>= 1;
        cpu_clock(cpu);
        cpu->Z = (cpu->temp_byte == 0) ? 1 : 0;
        cpu->N = (cpu->temp_byte & 0x80) ? 1 : 0;
        cpu_write_byte(cpu, cpu->temp_word, cpu->temp_byte);
        if (cpu->current_mode == 1 || cpu->current_mode == 2)
            cpu_clock(cpu);
    }
    cpu_clock(cpu);
    return 0;
}

Byte NOP(CPU *cpu){
    cpu_clock(cpu);
    return 0;
}

Byte ORA(CPU *cpu){
    if (cpu->current_mode == 1 || cpu->current_mode == 2 || cpu->current_mode == 3) cpu->temp_byte = cpu_read_byte(cpu, cpu->temp_word);
    cpu->A |= cpu->temp_byte;
    if (cpu->current_mode == 1)
        cpu_clock(cpu);
    cpu->Z = (cpu->A == 0) ? 1 : 0;
    cpu->N = (cpu->A & 0x80) ? 1 : 0;
    cpu_clock(cpu);
    return 0;
}

Byte PHA(CPU *cpu){
    stack_push(cpu, cpu->A);
    cpu_clock(cpu);
    cpu_clock(cpu);
    return 0;
}

Byte PHP(CPU *cpu){
    Byte SR = 0;
    SR |= (cpu->N) ? 0x80 : 0;
    SR |= (cpu->V) ? 0x40 : 0;
    SR |= 0x20;
    SR |= (cpu->B) ? 0x10 : 0;
    SR |= (cpu->D) ? 0x8 : 0;
    SR |= 0x4; // Interrupt flag set to 1
    SR |= (cpu->Z) ? 0x2 : 0;
    SR |= (cpu->C) ? 0x1 : 0;
    cpu_clock(cpu);
    stack_push(cpu, SR);
    cpu_clock(cpu);
    return 0;
}

Byte PLA(CPU *cpu){
    cpu_clock(cpu);
    cpu->A = stack_pop(cpu);
    cpu_clock(cpu);
    cpu->Z = (cpu->A == 0) ? 1 : 0;
    cpu->N = (cpu->A & 0x80) ? 1 : 0;
    cpu_clock(cpu);
    return 0;
}

Byte PLP(CPU *cpu){
    cpu_clock(cpu);
    Byte PS = stack_pop(cpu);
    cpu_clock(cpu);
    cpu->N = (PS & 0x80) ? 1 : 0;
    cpu->V = (PS & 0x40) ? 1 : 0;
    cpu->B = (PS & 0x10) ? 1 : 0;
    cpu->D = (PS & 0x8) ? 1 : 0;
    cpu->I = (PS & 0x4) ? 1 : 0;
    cpu->Z = (PS & 0x2) ? 1 : 0;
    cpu->C = (PS & 1) ? 1 : 0;
    cpu_clock(cpu);
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
        if (cpu->current_mode == 1 || cpu->current_mode == 2 || cpu->current_mode == 3) cpu->temp_byte = cpu_read_byte(cpu, cpu->temp_word);
        cpu->C = (cpu->temp_byte & 0x80) ? 1 : 0;
        cpu_clock(cpu);
        cpu->temp_byte <<= 1;
        cpu_clock(cpu);
        cpu->temp_byte |= cpu->C;
        cpu->Z = (cpu->temp_byte == 0) ? 1 : 0;
        cpu->N = (cpu->temp_byte & 0x80) ? 1 : 0;
        cpu_write_byte(cpu, cpu->temp_word, cpu->temp_byte);
        if (cpu->current_mode == 1 || cpu->current_mode == 2)
            cpu_clock(cpu);
    }
    cpu_clock(cpu);
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
        if (cpu->current_mode == 1 || cpu->current_mode == 2 || cpu->current_mode == 3) cpu->temp_byte = cpu_read_byte(cpu, cpu->temp_word);
        cpu->C = (cpu->temp_byte & 1) ? 1 : 0;
        cpu_clock(cpu);
        cpu->temp_byte >>= 1;
        cpu_clock(cpu);
        cpu->temp_byte |= (cpu->C) ? 0x80 : 0;
        cpu->Z = (cpu->temp_byte == 0) ? 1 : 0;
        cpu->N = (cpu->temp_byte & 0x80) ? 1 : 0;
        cpu_write_byte(cpu, cpu->temp_word, cpu->temp_byte);
        if (cpu->current_mode == 1 || cpu->current_mode == 2)
            cpu_clock(cpu);
    }
    cpu_clock(cpu);
    return 0;
}

Byte RTI(CPU *cpu){
    Byte PS = stack_pop(cpu);
    cpu_clock(cpu);
    cpu->N = (PS & 0x80) ? 1 : 0;
    cpu->V = (PS & 0x40) ? 1 : 0;
    cpu->B = (PS & 0x10) ? 1 : 0;
    cpu->D = (PS & 0x8) ? 1 : 0;
    cpu->I = (PS & 0x4) ? 1 : 0;
    cpu->Z = (PS & 0x2) ? 1 : 0;
    cpu->C = (PS & 1) ? 1 : 0;
    cpu_clock(cpu);
    Word new_PC = (Word) stack_pop(cpu);
    cpu_clock(cpu);
    new_PC |= ((Word) stack_pop(cpu)) << 8;
    cpu_clock(cpu);
    cpu->PC = new_PC;
    cpu_clock(cpu);
    return 0;
}

Byte RTS(CPU *cpu){
    cpu_clock(cpu);
    Word new_PC = (Word) stack_pop(cpu);
    cpu_clock(cpu);
    new_PC |= ((Word) stack_pop(cpu)) << 8;
    cpu_clock(cpu);
    cpu_clock(cpu);
    cpu->PC = new_PC + 1;
    cpu_clock(cpu);
    return 0;
}

Byte SBC(CPU *cpu){
    if (cpu->current_mode == 1 || cpu->current_mode == 2 || cpu->current_mode == 3) cpu->temp_byte = cpu_read_byte(cpu, cpu->temp_word);
    Word acc = (Word) cpu->A;
    Word tmp_byte = ((Word) cpu->temp_byte) ^ 0x00FF;
    Word temp_sum = acc + tmp_byte + cpu->C;

    cpu->A = (Byte) temp_sum;
    if (cpu->current_mode == 1)
        cpu_clock(cpu);
    cpu->C = (temp_sum > 256) ? 1 : 0;
    cpu->Z = (temp_sum == 0) ? 1 : 0;
    cpu->V = (~(acc ^ tmp_byte) & (acc ^ temp_sum) & 0x80) ? 1 : 0;
    cpu->N = (temp_sum & 0x80) ? 1 : 0;
    cpu_clock(cpu);
    return 0;
}

Byte SEC(CPU *cpu){
    cpu->C = 1;
    cpu_clock(cpu);
    return 0;
}

Byte SED(CPU *cpu){
    cpu->D = 1;
    cpu_clock(cpu);
    return 0;
}

Byte SEI(CPU *cpu){
    cpu->I = 1;
    cpu_clock(cpu);
    return 0;
}

Byte STA(CPU *cpu){
    if (cpu->current_mode == 1 || cpu->current_mode == 2 || cpu->current_mode == 3)
        cpu_clock(cpu);
    cpu_write_byte(cpu, cpu->temp_word, cpu->A);
    cpu_clock(cpu);
    return 0;
}

Byte STX(CPU *cpu){
    cpu_write_byte(cpu, cpu->temp_word, cpu->X);
    if (cpu->current_mode == 1)
        cpu_clock(cpu);
    cpu_clock(cpu);
    return 0;
}

Byte STY(CPU *cpu){
    cpu_write_byte(cpu, cpu->temp_word, cpu->Y);
    if (cpu->current_mode == 1)
        cpu_clock(cpu);
    cpu_clock(cpu);
    return 0;
}

Byte TAX(CPU *cpu){
    cpu->X = cpu->A;
    cpu->Z = (cpu->X == 0) ? 1 : 0;
    cpu->N = (cpu->X & 0x80) ? 1 : 0;
    cpu_clock(cpu);
    return 0;
}

Byte TAY(CPU *cpu){
    cpu->Y = cpu->A;
    cpu->Z = (cpu->Y == 0) ? 1 : 0;
    cpu->N = (cpu->Y & 0x80) ? 1 : 0;
    cpu_clock(cpu);
    return 0;
}

Byte TSX(CPU *cpu){
    cpu->X = (Byte) cpu->SP;
    cpu->Z = (cpu->X == 0) ? 1 : 0;
    cpu->N = (cpu->X & 0x80) ? 1 : 0;
    cpu_clock(cpu);
    return 0;
}

Byte TXA(CPU *cpu){
    cpu->A = cpu->X;
    cpu->Z = (cpu->X == 0) ? 1 : 0;
    cpu->N = (cpu->X & 0x80) ? 1 : 0;
    cpu_clock(cpu);
    return 0;
}

Byte TXS(CPU *cpu){
    cpu->SP = (Word) cpu->X;
    cpu->SP |= 0x100;
    cpu_clock(cpu);
    return 0;
}

Byte TYA(CPU *cpu){
    cpu->A = cpu->Y;
    cpu->Z = (cpu->Y == 0) ? 1 : 0;
    cpu->N = (cpu->Y & 0x80) ? 1 : 0;
    cpu_clock(cpu);
    return 0;
}

Byte NUL(CPU *cpu){
    return 0;
}


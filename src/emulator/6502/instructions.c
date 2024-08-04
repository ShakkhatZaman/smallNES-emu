#include "../global.h"
#include "instructions.h"

/*Helper functions */

static Byte cpu_read_byte(Word address);
static Byte cpu_write_byte(Word address, Byte data);
static void stack_push(Byte data);
static Byte stack_pop(void);
static void _set_status_A(void);
static void _check_page_crossed(void);

Byte fetch_byte(void) {
    Word counter = p_cpu->PC;
    p_cpu->PC = (counter == 0xFFFF) ? 0x8000 : counter + 1;
    Byte data = 0x00;
    if (counter >= 0x4020)
        data = p_mapper->cpu_read(p_mapper, counter);
    return data;
}

Word fetch_word(void) {
    Word counter = p_cpu->PC;
    Word data = 0x0000;
    p_cpu->PC = (counter == 0xFFFF) ? 0x8001 : counter + 2;
    if (counter >= 0x4020) {
        data = (Word) p_mapper->cpu_read(p_mapper, counter);
        cpu_clock();
        data |= ((Word) p_mapper->cpu_read(p_mapper, counter + 1)) << 8;
    }
    return data;
}

static Byte cpu_read_byte(Word address) {
    Byte data = 0x00;
    // Address inside CPU RAM
    if (address <= 0x1FFF)
        data = p_cpu->Bus.RAM[address & 0x07FF];    // 2KB of RAM mirrored across 8KB

    // Address inside PPU registers
    else if (address >= 0x2000 && address <= 0x3FFF)
        data = cpu_to_ppu_read(address);            // Reading on the ppu registers

        // Address inside cartridge
    else
        data = p_mapper->cpu_read(p_mapper, address);

    return data;
}

static Byte cpu_write_byte(Word address, Byte data) {
    // Address inside CPU RAM
    if (address <= 0x1FFF)
        p_cpu->Bus.RAM[address & 0x07FF] = data;         // 2KB of RAM mirrored across 8KB

        // Address inside PPU registers
    else if (address >= 0x2000 && address <= 0x3FFF)
        cpu_to_ppu_write(address, data);    // Writting on the ppu registers

        // Address inside cartridge
    else
        p_mapper->cpu_write(p_mapper, address, data);

    return 0;
}

static void stack_push(Byte data) {
    p_cpu->Bus.RAM[p_cpu->SP] = data;
    p_cpu->SP = (p_cpu->SP == 0x0100) ? 0x01FF : p_cpu->SP - 1;
}

static Byte stack_pop(void) {
    p_cpu->SP = (p_cpu->SP == 0x01FF) ? 0x0100 : p_cpu->SP + 1;
    return p_cpu->Bus.RAM[p_cpu->SP];
}

static void _set_status_A(void) {
    p_cpu->Z = (p_cpu->A == 0); // Zero flag is set if A is Zero
    p_cpu->N = (p_cpu->A & 0b10000000) ? 1 : 0;
}

static void _check_page_crossed(void) {
    if ((p_cpu->PC & 0xFF00) == (p_cpu->temp_word & 0xFF00))
        cpu_clock();
    else {
        cpu_clock();
        cpu_clock();
    }
    p_cpu->PC = p_cpu->temp_word;
}

void cpu_irq(void) {
    if (!(p_cpu->I)) {
        p_cpu->PC += 1;
        cpu_clock();
        stack_push((Byte) (p_cpu->PC >> 8));
        cpu_clock();
        stack_push((Byte) p_cpu->PC);
        cpu_clock();
        Byte SR = 0;
        SR |= (p_cpu->N) ? 0x80 : 0;
        SR |= (p_cpu->V) ? 0x40 : 0;
        SR |= (p_cpu->D) ? 0x8 : 0;
        SR |= (p_cpu->I) ? 0x4 : 0;
        SR |= (p_cpu->Z) ? 0x2 : 0;
        SR |= (p_cpu->C) ? 0x1 : 0;
        stack_push(SR);
        cpu_clock();
        p_cpu->I = 1;
        Word new_address = cpu_read_byte(0xFFFE);
        cpu_clock();
        new_address |= ((Word) cpu_read_byte(0xFFFF)) << 8;
        p_cpu->PC = new_address;
        cpu_clock();
    }
}

void cpu_nmi(void) {
    p_cpu->PC += 1;
    cpu_clock();
    stack_push((Byte) (p_cpu->PC >> 8));
    cpu_clock();
    stack_push((Byte) p_cpu->PC);
    cpu_clock();
    Byte SR = 0;
    SR |= (p_cpu->N) ? 0x80 : 0;
    SR |= (p_cpu->V) ? 0x40 : 0;
    SR |= (p_cpu->D) ? 0x8 : 0;
    SR |= (p_cpu->I) ? 0x4 : 0;
    SR |= (p_cpu->Z) ? 0x2 : 0;
    SR |= (p_cpu->C) ? 0x1 : 0;
    stack_push(SR);
    cpu_clock();
    p_cpu->I = 1;
    Word new_address = cpu_read_byte(0xFFFA);
    cpu_clock();
    new_address |= ((Word) cpu_read_byte(0xFFFB)) << 8;
    p_cpu->PC = new_address;
    cpu_clock();
}

const Ins ins_table[256] = {
    /*\--- 0 ---\ \--- 1 ---\ \--- 2 ---\ \--- 3 ---\ \--- 4 ---\ \--- 5 ---\ \--- 6 ---\ \--- 7 ---\ \--- 8 ---\ \--- 9 ---\ \--- A ---\ \--- B ---\ \--- C ---\ \--- D ---\ \--- E ---\ \--- F ---\*/
    /*= 0 =*/    {IMP, BRK}, {IZX, ORA}, {NUL, NUL}, {NUL, NUL}, {NUL, NUL}, {ZP0, ORA}, {ZP0, ASL}, {NUL, NUL}, {IMP, PHP}, {IMM, ORA}, {ACC, ASL}, {NUL, NUL}, {NUL, NUL}, {ABS, ORA}, {ABS, ASL}, {NUL, NUL},
    /*= 1 =*/    {REL, BPL}, {IZY, ORA}, {NUL, NUL}, {NUL, NUL}, {NUL, NUL}, {ZPX, ORA}, {ZPX, ASL}, {NUL, NUL}, {IMP, CLC}, {ABX, ORA}, {NUL, NUL}, {NUL, NUL}, {NUL, NUL}, {ABX, ORA}, {ABX, ASL}, {NUL, NUL},
    /*= 2 =*/    {ABS, JSR}, {IZX, AND}, {NUL, NUL}, {NUL, NUL}, {ZP0, BIT}, {ZP0, AND}, {ZP0, ROL}, {NUL, NUL}, {IMP, PLP}, {IMM, AND}, {ACC, ROL}, {NUL, NUL}, {ABS, BIT}, {ABS, AND}, {ABS, ROL}, {NUL, NUL},
    /*= 3 =*/    {REL, BMI}, {IZY, AND}, {NUL, NUL}, {NUL, NUL}, {NUL, NUL}, {ZPX, AND}, {ZPX, ROL}, {NUL, NUL}, {IMP, SEC}, {ABY, AND}, {NUL, NUL}, {NUL, NUL}, {NUL, NUL}, {ABX, AND}, {ABX, ROL}, {NUL, NUL},
    /*= 4 =*/    {IMP, RTI}, {IZX, EOR}, {NUL, NUL}, {NUL, NUL}, {NUL, NUL}, {ZP0, EOR}, {ZP0, LSR}, {NUL, NUL}, {IMP, PHA}, {IMM, EOR}, {ACC, LSR}, {NUL, NUL}, {ABS, JMP}, {ABS, EOR}, {ABS, LSR}, {NUL, NUL},
    /*= 5 =*/    {REL, BVC}, {IZY, EOR}, {NUL, NUL}, {NUL, NUL}, {NUL, NUL}, {ZPX, EOR}, {ZPX, LSR}, {NUL, NUL}, {IMP, CLI}, {ABY, EOR}, {NUL, NUL}, {NUL, NUL}, {NUL, NUL}, {ABX, EOR}, {ABX, LSR}, {NUL, NUL},
    /*= 6 =*/    {IMP, RTS}, {IZX, ADC}, {NUL, NUL}, {NUL, NUL}, {NUL, NUL}, {ZP0, ADC}, {ZP0, ROR}, {NUL, NUL}, {IMP, PLA}, {IMM, ADC}, {ACC, ROR}, {NUL, NUL}, {IND, JMP}, {ABS, ADC}, {ABS, ROR}, {NUL, NUL},
    /*= 7 =*/    {REL, BVS}, {IZY, ADC}, {NUL, NUL}, {NUL, NUL}, {NUL, NUL}, {ZPX, ADC}, {ZPX, ROR}, {NUL, NUL}, {IMP, SEI}, {ABY, ADC}, {NUL, NUL}, {NUL, NUL}, {NUL, NUL}, {ABX, ADC}, {ABX, ROR}, {NUL, NUL},
    /*= 8 =*/    {NUL, NUL}, {IZX, STA}, {NUL, NUL}, {NUL, NUL}, {ZP0, STY}, {ZP0, STA}, {ZP0, STX}, {NUL, NUL}, {IMP, DEY}, {NUL, NUL}, {IMP, TXA}, {NUL, NUL}, {ABS, STY}, {ABS, STA}, {ABS, STX}, {NUL, NUL},
    /*= 9 =*/    {REL, BCC}, {IZY, STA}, {NUL, NUL}, {NUL, NUL}, {ZPX, STY}, {ZPX, STA}, {ZPY, STX}, {NUL, NUL}, {IMP, TYA}, {ABY, STA}, {IMP, TXS}, {NUL, NUL}, {NUL, NUL}, {ABX, STA}, {NUL, NUL}, {NUL, NUL},
    /*= A =*/    {IMM, LDY}, {IZX, LDA}, {IMM, LDX}, {NUL, NUL}, {ZP0, LDY}, {ZP0, LDA}, {ZP0, LDX}, {NUL, NUL}, {IMP, TAY}, {IMM, LDA}, {IMP, TAX}, {NUL, NUL}, {ABS, LDY}, {ABS, LDA}, {ABS, LDX}, {NUL, NUL},
    /*= B =*/    {REL, BCS}, {IZY, LDA}, {NUL, NUL}, {NUL, NUL}, {ZPX, LDY}, {ZPX, LDA}, {ZPY, LDX}, {NUL, NUL}, {IMP, CLV}, {ABY, LDA}, {IMP, TSX}, {NUL, NUL}, {ABX, LDY}, {ABX, LDA}, {ABY, LDX}, {NUL, NUL},
    /*= C =*/    {IMM, CPY}, {IZX, CMP}, {NUL, NUL}, {NUL, NUL}, {ZP0, CPY}, {ZP0, CMP}, {ZP0, DEC}, {NUL, NUL}, {IMP, INY}, {IMM, CMP}, {IMP, DEX}, {NUL, NUL}, {ABS, CPY}, {ABS, CMP}, {ABS, DEC}, {NUL, NUL},
    /*= D =*/    {REL, BNE}, {IZY, CMP}, {NUL, NUL}, {NUL, NUL}, {NUL, NUL}, {ZPX, CMP}, {ZPX, DEC}, {NUL, NUL}, {IMP, CLD}, {ABY, CMP}, {NUL, NUL}, {NUL, NUL}, {NUL, NUL}, {ABX, CMP}, {ABX, DEC}, {NUL, NUL},
    /*= E =*/    {IMM, CPX}, {IZX, SBC}, {NUL, NUL}, {NUL, NUL}, {ZP0, CPX}, {ZP0, SBC}, {ZP0, INC}, {NUL, NUL}, {IMP, INX}, {IMM, SBC}, {IMP, NOP}, {NUL, NUL}, {ABS, CPX}, {ABS, SBC}, {ABS, INC}, {NUL, NUL},
    /*= F =*/    {REL, BEQ}, {IZY, SBC}, {NUL, NUL}, {NUL, NUL}, {NUL, NUL}, {ZPX, SBC}, {ZPX, INC}, {NUL, NUL}, {IMP, SED}, {ABY, SBC}, {NUL, NUL}, {NUL, NUL}, {NUL, NUL}, {ABX, SBC}, {ABX, INC}, {NUL, NUL}
};

/*Addressing modes*/

Byte ABS(void) {
    Word data_address = fetch_word();
    p_cpu->temp_word = data_address;
    p_cpu->current_mode = 1;
    return 0;
}

Byte ABX(void) {
    Word data_address = fetch_word();
    cpu_clock();
    Word data_address_x = data_address + p_cpu->X;
    p_cpu->temp_word = data_address_x;
    p_cpu->current_mode = 2;
    if ((data_address & 0xFF00) == (data_address_x & 0xFF00))
        return 0;
    else{
        cpu_clock();
        return 1;
    }
}

Byte ABY(void) {
    Word data_address = fetch_word();
    cpu_clock();
    Word data_address_y = data_address + p_cpu->Y;
    p_cpu->temp_word = data_address_y;
    p_cpu->current_mode = 3;
    if ((data_address & 0xFF00) == (data_address_y & 0xFF00))
        return 0;
    else{
        cpu_clock();
        return 1;
    }
}

Byte ACC(void) {
    p_cpu->temp_byte = p_cpu->A;
    p_cpu->current_mode = 4;
    return 0;
}

Byte IMP(void) {
    p_cpu->current_mode = 5;
    return 0;
}

Byte IMM(void) {
    p_cpu->temp_byte = fetch_byte();
    p_cpu->current_mode = 6;
    return 0;
}

Byte IND(void) {
    Word data_address = fetch_word();
    Byte byte_data = cpu_read_byte(data_address);
    cpu_clock();
    Word word_data = 0;

    if ((data_address & 0x00FF) == 0x00FF) {
        word_data = (((Word) cpu_read_byte(data_address & 0xFF00)) << 8) | ((Word) byte_data);
        cpu_clock();
    }
    else {
        word_data = (((Word) cpu_read_byte(data_address + 1)) << 8) | ((Word) byte_data);
        cpu_clock();
    }
    p_cpu->temp_word = word_data;
    p_cpu->temp_byte = cpu_read_byte(word_data);
    p_cpu->current_mode = 7;
    return 0;
}

Byte IZX(void) {
    Byte zp_data_address_x = fetch_byte();
    cpu_clock();
    zp_data_address_x += p_cpu->X;
    Word full_zp_address_x = (Word) zp_data_address_x;
    cpu_clock();
    Byte byte_data = cpu_read_byte(full_zp_address_x);
    cpu_clock();
    Word word_data = (((Word) cpu_read_byte(full_zp_address_x + 1)) << 8) | ((Word) byte_data);
    cpu_clock();
    p_cpu->temp_word = word_data;
    p_cpu->temp_byte = cpu_read_byte(word_data);
    p_cpu->current_mode = 8;
    return 0;
}

Byte IZY(void) {
    Byte zp_data_address_y = fetch_byte();
    cpu_clock();
    Word full_zp_address_y = (Word) zp_data_address_y;
    Byte byte_data = cpu_read_byte(full_zp_address_y);
    cpu_clock();
    Word word_data = (((Word) cpu_read_byte(full_zp_address_y + 1)) << 8) | ((Word) byte_data);
    cpu_clock();
    Word temp_word_data = (Word) p_cpu->Y + word_data;
    p_cpu->temp_word = temp_word_data;
    p_cpu->temp_byte = cpu_read_byte(temp_word_data);
    p_cpu->current_mode = 9;
    if ((temp_word_data & 0xFF00) == (word_data & 0xFF00))
        return 0;
    else {
        cpu_clock();
        return 1;
    }
}

Byte REL(void) {
    Byte byte_offset = fetch_byte();
    p_cpu->temp_byte = byte_offset;
    Word target = p_cpu->PC;
    target += (int8_t) byte_offset;

    p_cpu->temp_word = target;
    p_cpu->current_mode = 10;
    return 0;
}

Byte ZP0(void) {
    Byte zero_page_address = fetch_byte();
    cpu_clock();
    p_cpu->temp_word = (Word) zero_page_address;
    p_cpu->temp_byte = cpu_read_byte((Word) zero_page_address);
    p_cpu->current_mode = 11;
    return 0;
}

Byte ZPX(void) {
    Byte zero_page_address_x = fetch_byte();
    cpu_clock();
    zero_page_address_x += p_cpu->X;
    p_cpu->temp_word = (Word) zero_page_address_x;
    cpu_clock();
    p_cpu->temp_byte = cpu_read_byte((Word) zero_page_address_x);
    p_cpu->current_mode = 12;
    return 0;
}

Byte ZPY(void) {
    Byte zero_page_address_y = fetch_byte();
    cpu_clock();
    zero_page_address_y += p_cpu->Y;
    p_cpu->temp_word = (Word) zero_page_address_y;
    cpu_clock();
    p_cpu->temp_byte = cpu_read_byte((Word) zero_page_address_y);
    p_cpu->current_mode = 13;
    return 0;
}

/*Operation functions*/

Byte ADC(void) {
    if (p_cpu->current_mode == 1 || p_cpu->current_mode == 2 || p_cpu->current_mode == 3) p_cpu->temp_byte = cpu_read_byte(p_cpu->temp_word);
    Word acc = (Word) p_cpu->A;
    Word tmp_byte = (Word) p_cpu->temp_byte;
    Word temp_sum = acc + tmp_byte + p_cpu->C;

    p_cpu->A = (Byte) temp_sum;
    if (p_cpu->current_mode == 1)
        cpu_clock();
    p_cpu->C = (temp_sum > 256) ? 1 : 0;
    p_cpu->Z = (temp_sum == 0) ? 1 : 0;
    p_cpu->V = (~(acc ^ tmp_byte) & (acc ^ temp_sum) & 0x80) ? 1 : 0;
    p_cpu->N = (temp_sum & 0x80) ? 1 : 0;
    cpu_clock();
    return 0;
}

Byte AND(void){
    if (p_cpu->current_mode == 1 || p_cpu->current_mode == 2 || p_cpu->current_mode == 3) p_cpu->temp_byte = cpu_read_byte(p_cpu->temp_word);
    if (p_cpu->current_mode == 1)
        cpu_clock();
    p_cpu->A &= p_cpu->temp_byte;
    p_cpu->Z = (p_cpu->A == 0) ? 1 : 0;
    p_cpu->N = (p_cpu->A & 0x80) ? 1 : 0;
    cpu_clock();
    return 0;
}

Byte ASL(void){
    if (p_cpu->current_mode == 4){
        p_cpu->C = (p_cpu->A & 0x80) ? 1 : 0;
        p_cpu->A <<= 1;
        p_cpu->Z = (p_cpu->A == 0) ? 1 : 0;
        p_cpu->N = (p_cpu->A & 0x80) ? 1 : 0;
    }
    else {
        if (p_cpu->current_mode == 1 || p_cpu->current_mode == 2 || p_cpu->current_mode == 3) p_cpu->temp_byte = cpu_read_byte(p_cpu->temp_word);
        p_cpu->C = (p_cpu->temp_byte & 0x80) ? 1 : 0;
        cpu_clock();
        p_cpu->temp_byte >>= 1;
        cpu_clock();
        p_cpu->Z = (p_cpu->temp_byte == 0) ? 1 : 0;
        p_cpu->N = (p_cpu->temp_byte & 0x80) ? 1 : 0;
        if (p_cpu->current_mode == 1 || p_cpu->current_mode == 2)
            cpu_clock();
        cpu_write_byte(p_cpu->temp_word, p_cpu->temp_byte);
    }
    cpu_clock();
    return 0;
}

Byte BCC(void){
    if (!p_cpu->C)
        _check_page_crossed();
    cpu_clock();
    return 0;
}

Byte BCS(void){
    if (p_cpu->C)
        _check_page_crossed();
    cpu_clock();
    return 0;
}

Byte BEQ(void){
    if (p_cpu->Z)
        _check_page_crossed();

    cpu_clock();
    return 0;
}

Byte BIT(void){
    if (p_cpu->current_mode == 1)
        cpu_clock();
    if (p_cpu->current_mode == 1 || p_cpu->current_mode == 2 || p_cpu->current_mode == 3) p_cpu->temp_byte = cpu_read_byte(p_cpu->temp_word);
    p_cpu->N = (p_cpu->temp_byte & 0b10000000) ? 1 : 0;
    p_cpu->V = (p_cpu->temp_byte & 0b01000000) ? 1 : 0;
    p_cpu->Z = (p_cpu->temp_byte & p_cpu->A) ? 0 : 1;
    cpu_clock();
    return 0;
}

Byte BMI(void){
    if (p_cpu->N)
        _check_page_crossed();
    cpu_clock();
    return 0;
}

Byte BNE(void){
    if (!p_cpu->Z)
        _check_page_crossed();
    cpu_clock();
    return 0;
}

Byte BPL(void){
    if (!p_cpu->N)
        p_cpu->PC = p_cpu->temp_word;
    cpu_clock();
    return 0;
}

Byte BRK(void){
    p_cpu->PC += 1;
    stack_push((Byte) (p_cpu->PC >> 8));
    cpu_clock();
    stack_push((Byte) (p_cpu->PC));
    cpu_clock();
    Byte SR = 0;
    SR |= (p_cpu->N) ? 0x80 : 0;
    SR |= (p_cpu->V) ? 0x40 : 0;
    SR |= 0x10;
    SR |= (p_cpu->D) ? 0x8 : 0;
    SR |= (p_cpu->I) ? 0x4 : 0;
    SR |= (p_cpu->Z) ? 0x2 : 0;
    SR |= (p_cpu->C) ? 0x1 : 0;
    stack_push(SR);
    cpu_clock();
    p_cpu->I = 1;
    cpu_clock();
    Word new_address = cpu_read_byte(0xFFFE);
    cpu_clock();
    new_address |= ((Word) cpu_read_byte(0xFFFF)) << 8;
    p_cpu->PC = new_address;
    cpu_clock();
    return 0;
}

Byte BVC(void){
    if (!p_cpu->V)
        _check_page_crossed();
    cpu_clock();
    return 0;
}

Byte BVS(void){
    if (p_cpu->V)
        _check_page_crossed();
    cpu_clock();
    return 0;
}

Byte CLC(void){
    p_cpu->C = 0;
    cpu_clock();
    return 0;
}

Byte CLD(void){
    p_cpu->D = 0;
    cpu_clock();
    return 0;
}

Byte CLI(void){
    p_cpu->I = 0;
    cpu_clock();
    return 0;
}

Byte CLV(void){
    p_cpu->V = 0;
    cpu_clock();
    return 0;
}

Byte CMP(void){
    if (p_cpu->current_mode == 1 || p_cpu->current_mode == 2 || p_cpu->current_mode == 3) p_cpu->temp_byte = cpu_read_byte(p_cpu->temp_word);
    Word a = (Word) p_cpu->A;
    Word mem = (Word) p_cpu->temp_byte;
    Word result = a - mem;
    if (p_cpu->current_mode == 1)
        cpu_clock();
    p_cpu->C = (a >= mem) ? 1 : 0;
    p_cpu->Z = ((result & 0xFF) == 0) ? 1 : 0;
    p_cpu->N = (result & 0x80) ? 1 : 0;
    cpu_clock();
    return 0;
}

Byte CPX(void){
    if (p_cpu->current_mode == 1 || p_cpu->current_mode == 2 || p_cpu->current_mode == 3) p_cpu->temp_byte = cpu_read_byte(p_cpu->temp_word);
    Word x = (Word) p_cpu->X;
    Word mem = (Word) p_cpu->temp_byte;
    Word result = x - mem;
    if (p_cpu->current_mode == 1)
        cpu_clock();
    p_cpu->C = (x >= mem) ? 1 : 0;
    p_cpu->Z = ((result & 0xFF) == 0) ? 1 : 0;
    p_cpu->N = (result & 0x80) ? 1 : 0;
    cpu_clock();
    return 0;
}

Byte CPY(void){
    if (p_cpu->current_mode == 1 || p_cpu->current_mode == 2 || p_cpu->current_mode == 3) p_cpu->temp_byte = cpu_read_byte(p_cpu->temp_word);
    Word y = (Word) p_cpu->Y;
    Word mem = (Word) p_cpu->temp_byte;
    Word result = y - mem;
    if (p_cpu->current_mode == 1)
        cpu_clock();
    p_cpu->C = (y >= mem) ? 1 : 0;
    p_cpu->Z = ((result & 0xFF) == 0) ? 1 : 0;
    p_cpu->N = (result & 0x80) ? 1 : 0;
    cpu_clock();
    return 0;
}

Byte DEC(void){
    if (p_cpu->current_mode == 1 || p_cpu->current_mode == 2 || p_cpu->current_mode == 3) p_cpu->temp_byte = cpu_read_byte(p_cpu->temp_word);
    p_cpu->temp_byte -= 1; 
    cpu_clock();
    cpu_write_byte(p_cpu->temp_word, p_cpu->temp_byte);
    cpu_clock();
    if (p_cpu->current_mode == 1 || p_cpu->current_mode == 2)
        cpu_clock();
    p_cpu->Z = (p_cpu->temp_byte == 0) ? 1 : 0;
    p_cpu->N = (p_cpu->temp_byte & 0x80) ? 1 : 0;
    cpu_clock();
    return 0;
}

Byte DEX(void){
    p_cpu->X -= 1; 
    p_cpu->Z = (p_cpu->X == 0) ? 1 : 0;
    p_cpu->N = (p_cpu->X & 0x80) ? 1 : 0;
    cpu_clock();
    return 0;
}

Byte DEY(void){
    p_cpu->Y -= 1; 
    p_cpu->Z = (p_cpu->Y == 0) ? 1 : 0;
    p_cpu->N = (p_cpu->Y & 0x80) ? 1 : 0;
    cpu_clock();
    return 0;
}

Byte EOR(void){
    if (p_cpu->current_mode == 1 || p_cpu->current_mode == 2 || p_cpu->current_mode == 3) p_cpu->temp_byte = cpu_read_byte(p_cpu->temp_word);
    p_cpu->A ^= p_cpu->temp_byte;
    if (p_cpu->current_mode == 1)
        cpu_clock();
    p_cpu->Z = (p_cpu->A == 0) ? 1 : 0;
    p_cpu->N = (p_cpu->A & 0x80) ? 1 : 0;
    cpu_clock();
    return 0;
}

Byte INC(void){
    if (p_cpu->current_mode == 1 || p_cpu->current_mode == 2 || p_cpu->current_mode == 3) p_cpu->temp_byte = cpu_read_byte(p_cpu->temp_word);
    p_cpu->temp_byte += 1; 
    cpu_clock();
    cpu_write_byte(p_cpu->temp_word, p_cpu->temp_byte);
    cpu_clock();
    if (p_cpu->current_mode == 1 || p_cpu->current_mode == 2)
        cpu_clock();
    p_cpu->Z = (p_cpu->temp_byte == 0) ? 1 : 0;
    p_cpu->N = (p_cpu->temp_byte & 0x80) ? 1 : 0;
    cpu_clock();
    return 0;
}

Byte INX(void){
    p_cpu->X += 1; 
    p_cpu->Z = (p_cpu->X == 0) ? 1 : 0;
    p_cpu->N = (p_cpu->X & 0x80) ? 1 : 0;
    cpu_clock();
    return 0;
}

Byte INY(void){
    p_cpu->Y += 1; 
    p_cpu->Z = (p_cpu->Y == 0) ? 1 : 0;
    p_cpu->N = (p_cpu->Y & 0x80) ? 1 : 0;
    cpu_clock();
    return 0;
}

Byte JMP(void){
    p_cpu->PC = p_cpu->temp_word;
    cpu_clock();
    return 0;
}

Byte JSR(void){
    Word current_pc = p_cpu->PC - 1;
    cpu_clock();
    stack_push((Byte) (current_pc >> 8));
    cpu_clock();
    stack_push((Byte) current_pc);
    cpu_clock();
    p_cpu->PC = p_cpu->temp_word;
    cpu_clock();
    return 0;
}

Byte LDA(void){
    if (p_cpu->current_mode == 1 || p_cpu->current_mode == 2 || p_cpu->current_mode == 3) p_cpu->temp_byte = cpu_read_byte(p_cpu->temp_word);
    p_cpu->A = p_cpu->temp_byte;
    if (p_cpu->current_mode == 1)
        cpu_clock();
    _set_status_A();
    cpu_clock();
    return 0;
}

Byte LDX(void){
    if (p_cpu->current_mode == 1 || p_cpu->current_mode == 2 || p_cpu->current_mode == 3) p_cpu->temp_byte = cpu_read_byte(p_cpu->temp_word);
    p_cpu->X = p_cpu->temp_byte;
    if (p_cpu->current_mode == 1)
        cpu_clock();
    p_cpu->Z = (p_cpu->X == 0) ? 1 : 0;
    p_cpu->N = (p_cpu->X & 0x80) ? 1 : 0;
    cpu_clock();
    return 0;
}

Byte LDY(void){
    if (p_cpu->current_mode == 1 || p_cpu->current_mode == 2 || p_cpu->current_mode == 3) p_cpu->temp_byte = cpu_read_byte(p_cpu->temp_word);
    p_cpu->Y = p_cpu->temp_byte;
    if (p_cpu->current_mode == 1)
        cpu_clock();
    p_cpu->Z = (p_cpu->Y == 0) ? 1 : 0;
    p_cpu->N = (p_cpu->Y & 0x80) ? 1 : 0;
    cpu_clock();
    return 0;
}

Byte LSR(void){
    if (p_cpu->current_mode == 4){
        p_cpu->C = (p_cpu->A & 1) ? 1 : 0;
        p_cpu->A = p_cpu->A >> 1;
        p_cpu->Z = (p_cpu->A == 0) ? 1 : 0;
        p_cpu->N = (p_cpu->A & 0x80) ? 1 : 0;
    }
    else {
        if (p_cpu->current_mode == 1 || p_cpu->current_mode == 2 || p_cpu->current_mode == 3) p_cpu->temp_byte = cpu_read_byte(p_cpu->temp_word);
        p_cpu->C = (p_cpu->temp_byte & 1) ? 1 : 0;
        cpu_clock();
        p_cpu->temp_byte >>= 1;
        cpu_clock();
        p_cpu->Z = (p_cpu->temp_byte == 0) ? 1 : 0;
        p_cpu->N = (p_cpu->temp_byte & 0x80) ? 1 : 0;
        cpu_write_byte(p_cpu->temp_word, p_cpu->temp_byte);
        if (p_cpu->current_mode == 1 || p_cpu->current_mode == 2)
            cpu_clock();
    }
    cpu_clock();
    return 0;
}

Byte NOP(void){
    cpu_clock();
    return 0;
}

Byte ORA(void){
    if (p_cpu->current_mode == 1 || p_cpu->current_mode == 2 || p_cpu->current_mode == 3) p_cpu->temp_byte = cpu_read_byte(p_cpu->temp_word);
    p_cpu->A |= p_cpu->temp_byte;
    if (p_cpu->current_mode == 1)
        cpu_clock();
    p_cpu->Z = (p_cpu->A == 0) ? 1 : 0;
    p_cpu->N = (p_cpu->A & 0x80) ? 1 : 0;
    cpu_clock();
    return 0;
}

Byte PHA(void){
    stack_push(p_cpu->A);
    cpu_clock();
    cpu_clock();
    return 0;
}

Byte PHP(void){
    Byte SR = 0;
    SR |= (p_cpu->N) ? 0x80 : 0;
    SR |= (p_cpu->V) ? 0x40 : 0;
    SR |= 0x20;
    SR |= (p_cpu->B) ? 0x10 : 0;
    SR |= (p_cpu->D) ? 0x8 : 0;
    SR |= 0x4; // Interrupt flag set to 1
    SR |= (p_cpu->Z) ? 0x2 : 0;
    SR |= (p_cpu->C) ? 0x1 : 0;
    cpu_clock();
    stack_push(SR);
    cpu_clock();
    return 0;
}

Byte PLA(void){
    cpu_clock();
    p_cpu->A = stack_pop();
    cpu_clock();
    p_cpu->Z = (p_cpu->A == 0) ? 1 : 0;
    p_cpu->N = (p_cpu->A & 0x80) ? 1 : 0;
    cpu_clock();
    return 0;
}

Byte PLP(void){
    cpu_clock();
    Byte PS = stack_pop();
    cpu_clock();
    p_cpu->N = (PS & 0x80) ? 1 : 0;
    p_cpu->V = (PS & 0x40) ? 1 : 0;
    p_cpu->B = (PS & 0x10) ? 1 : 0;
    p_cpu->D = (PS & 0x8) ? 1 : 0;
    p_cpu->I = (PS & 0x4) ? 1 : 0;
    p_cpu->Z = (PS & 0x2) ? 1 : 0;
    p_cpu->C = (PS & 1) ? 1 : 0;
    cpu_clock();
    return 0;
}

Byte ROL(void){
    if (p_cpu->current_mode == 4){
        p_cpu->C = (p_cpu->A & 0x80) ? 1 : 0;
        p_cpu->A <<= 1;
        p_cpu->A |= p_cpu->C;
        p_cpu->Z = (p_cpu->A == 0) ? 1 : 0;
        p_cpu->N = (p_cpu->A & 0x80) ? 1 : 0;
    }
    else {
        if (p_cpu->current_mode == 1 || p_cpu->current_mode == 2 || p_cpu->current_mode == 3) p_cpu->temp_byte = cpu_read_byte(p_cpu->temp_word);
        p_cpu->C = (p_cpu->temp_byte & 0x80) ? 1 : 0;
        cpu_clock();
        p_cpu->temp_byte <<= 1;
        cpu_clock();
        p_cpu->temp_byte |= p_cpu->C;
        p_cpu->Z = (p_cpu->temp_byte == 0) ? 1 : 0;
        p_cpu->N = (p_cpu->temp_byte & 0x80) ? 1 : 0;
        cpu_write_byte(p_cpu->temp_word, p_cpu->temp_byte);
        if (p_cpu->current_mode == 1 || p_cpu->current_mode == 2)
            cpu_clock();
    }
    cpu_clock();
    return 0;
}

Byte ROR(void){
    if (p_cpu->current_mode == 4){
        p_cpu->C = (p_cpu->A & 1) ? 1 : 0;
        p_cpu->A >>= 1;
        p_cpu->A |= (p_cpu->C) ? 0x80 : 0;
        p_cpu->Z = (p_cpu->A == 0) ? 1 : 0;
        p_cpu->N = (p_cpu->A & 0x80) ? 1 : 0;
    }
    else {
        if (p_cpu->current_mode == 1 || p_cpu->current_mode == 2 || p_cpu->current_mode == 3) p_cpu->temp_byte = cpu_read_byte(p_cpu->temp_word);
        p_cpu->C = (p_cpu->temp_byte & 1) ? 1 : 0;
        cpu_clock();
        p_cpu->temp_byte >>= 1;
        cpu_clock();
        p_cpu->temp_byte |= (p_cpu->C) ? 0x80 : 0;
        p_cpu->Z = (p_cpu->temp_byte == 0) ? 1 : 0;
        p_cpu->N = (p_cpu->temp_byte & 0x80) ? 1 : 0;
        cpu_write_byte(p_cpu->temp_word, p_cpu->temp_byte);
        if (p_cpu->current_mode == 1 || p_cpu->current_mode == 2)
            cpu_clock();
    }
    cpu_clock();
    return 0;
}

Byte RTI(void){
    Byte PS = stack_pop();
    cpu_clock();
    p_cpu->N = (PS & 0x80) ? 1 : 0;
    p_cpu->V = (PS & 0x40) ? 1 : 0;
    p_cpu->B = (PS & 0x10) ? 1 : 0;
    p_cpu->D = (PS & 0x8) ? 1 : 0;
    p_cpu->I = (PS & 0x4) ? 1 : 0;
    p_cpu->Z = (PS & 0x2) ? 1 : 0;
    p_cpu->C = (PS & 1) ? 1 : 0;
    cpu_clock();
    Word new_PC = (Word) stack_pop();
    cpu_clock();
    new_PC |= ((Word) stack_pop()) << 8;
    cpu_clock();
    p_cpu->PC = new_PC;
    cpu_clock();
    return 0;
}

Byte RTS(void){
    cpu_clock();
    Word new_PC = (Word) stack_pop();
    cpu_clock();
    new_PC |= ((Word) stack_pop()) << 8;
    cpu_clock();
    cpu_clock();
    p_cpu->PC = new_PC + 1;
    cpu_clock();
    return 0;
}

Byte SBC(void){
    if (p_cpu->current_mode == 1 || p_cpu->current_mode == 2 || p_cpu->current_mode == 3) p_cpu->temp_byte = cpu_read_byte(p_cpu->temp_word);
    Word acc = (Word) p_cpu->A;
    Word tmp_byte = ((Word) p_cpu->temp_byte) ^ 0x00FF;
    Word temp_sum = acc + tmp_byte + p_cpu->C;

    p_cpu->A = (Byte) temp_sum;
    if (p_cpu->current_mode == 1)
        cpu_clock();
    p_cpu->C = (temp_sum > 256) ? 1 : 0;
    p_cpu->Z = (temp_sum == 0) ? 1 : 0;
    p_cpu->V = (~(acc ^ tmp_byte) & (acc ^ temp_sum) & 0x80) ? 1 : 0;
    p_cpu->N = (temp_sum & 0x80) ? 1 : 0;
    cpu_clock();
    return 0;
}

Byte SEC(void){
    p_cpu->C = 1;
    cpu_clock();
    return 0;
}

Byte SED(void){
    p_cpu->D = 1;
    cpu_clock();
    return 0;
}

Byte SEI(void){
    p_cpu->I = 1;
    cpu_clock();
    return 0;
}

Byte STA(void){
    if (p_cpu->current_mode == 1 || p_cpu->current_mode == 2 || p_cpu->current_mode == 3)
        cpu_clock();
    cpu_write_byte(p_cpu->temp_word, p_cpu->A);
    cpu_clock();
    return 0;
}

Byte STX(void){
    cpu_write_byte(p_cpu->temp_word, p_cpu->X);
    if (p_cpu->current_mode == 1)
        cpu_clock();
    cpu_clock();
    return 0;
}

Byte STY(void){
    cpu_write_byte(p_cpu->temp_word, p_cpu->Y);
    if (p_cpu->current_mode == 1)
        cpu_clock();
    cpu_clock();
    return 0;
}

Byte TAX(void){
    p_cpu->X = p_cpu->A;
    p_cpu->Z = (p_cpu->X == 0) ? 1 : 0;
    p_cpu->N = (p_cpu->X & 0x80) ? 1 : 0;
    cpu_clock();
    return 0;
}

Byte TAY(void){
    p_cpu->Y = p_cpu->A;
    p_cpu->Z = (p_cpu->Y == 0) ? 1 : 0;
    p_cpu->N = (p_cpu->Y & 0x80) ? 1 : 0;
    cpu_clock();
    return 0;
}

Byte TSX(void){
    p_cpu->X = (Byte) p_cpu->SP;
    p_cpu->Z = (p_cpu->X == 0) ? 1 : 0;
    p_cpu->N = (p_cpu->X & 0x80) ? 1 : 0;
    cpu_clock();
    return 0;
}

Byte TXA(void){
    p_cpu->A = p_cpu->X;
    p_cpu->Z = (p_cpu->X == 0) ? 1 : 0;
    p_cpu->N = (p_cpu->X & 0x80) ? 1 : 0;
    cpu_clock();
    return 0;
}

Byte TXS(void){
    p_cpu->SP = (Word) p_cpu->X;
    p_cpu->SP |= 0x100;
    cpu_clock();
    return 0;
}

Byte TYA(void){
    p_cpu->A = p_cpu->Y;
    p_cpu->Z = (p_cpu->Y == 0) ? 1 : 0;
    p_cpu->N = (p_cpu->Y & 0x80) ? 1 : 0;
    cpu_clock();
    return 0;
}

Byte NUL(void){
    return 0;
}


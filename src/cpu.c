#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cpu.h"

#define STACK_START    0x0100
#define STACK_END      0x01FF
#define STK_PTR_START  0xFD
#define NMI_LO         0xFFFA
#define NMI_HI         0xFFFB
#define RESET_LO       0xFFFC
#define RESET_HI       0xFFFD
#define IRQ_LO         0xFFFE
#define IRQ_HI         0xFFFF
#define N_INSTRUCTIONS 256
#define CPU_CLK_START  7


FILE *assembly_outfile;

// full instr set: https://www.masswerk.at/6502/6502_instruction_set.html
// credit to OneLoneCoder for the idea behind this instruction set representation
Instruction instruction_table[N_INSTRUCTIONS] = 
{// -0                          -1                                -2                          -3                      -4                              -5                              -6                              -7                      -8                        -9                             -A                            -B                      -C                             -D                             -E                             -F
	{"BRK", BRK, implied, 7},   {"ORA", ORA, zero_indirect_x, 6}, {"XXX", NULL, NULL, 2},     {"XXX", NULL, NULL, 2}, {"XXX", NULL, NULL, 2},         {"ORA", ORA, zero_page, 3},     {"ASL", ASL, zero_page, 5},     {"XXX", NULL, NULL, 2}, {"PHP", PHP, implied, 3}, {"ORA", ORA, immediate, 2},    {"ASL", ASL, accumulator, 2}, {"XXX", NULL, NULL, 2}, {"XXX", NULL, NULL, 2},        {"ORA", ORA, absolute, 4},     {"ASL", ASL, absolute, 6},     {"XXX", NULL, NULL, 2}, // 2-
	{"BPL", BPL, relative, 2},  {"ORA", ORA, zero_indirect_y, 5}, {"XXX", NULL, NULL, 2},     {"XXX", NULL, NULL, 2}, {"XXX", NULL, NULL, 2},         {"ORA", ORA, zero_offset_x, 4}, {"ASL", ASL, zero_offset_x, 6}, {"XXX", NULL, NULL, 2}, {"CLC", CLC, implied, 2}, {"ORA", ORA, abs_offset_y, 4}, {"XXX", NULL, NULL, 2},       {"XXX", NULL, NULL, 2}, {"XXX", NULL, NULL, 2},        {"ORA", ORA, abs_offset_x, 4}, {"ASL", ASL, abs_offset_x, 7}, {"XXX", NULL, NULL, 2}, // 1-
	{"JSR", JSR, absolute, 6},  {"AND", AND, zero_indirect_x, 6}, {"XXX", NULL, NULL, 2},     {"XXX", NULL, NULL, 2}, {"BIT", BIT, zero_page, 3},     {"AND", AND, zero_page, 3},     {"ROL", ROL, zero_page, 5},     {"XXX", NULL, NULL, 2}, {"PLP", PLP, implied, 4}, {"AND", AND, immediate, 2},    {"ROL", ROL, accumulator, 2}, {"XXX", NULL, NULL, 2}, {"BIT", BIT, absolute, 4},     {"AND", AND, absolute, 4},     {"ROL", ROL, absolute, 6},     {"XXX", NULL, NULL, 2}, // 2-
	{"BMI", BMI, relative, 2},  {"AND", AND, zero_indirect_y, 5}, {"XXX", NULL, NULL, 2},     {"XXX", NULL, NULL, 2}, {"XXX", NULL, NULL, 2},         {"AND", AND, zero_offset_x, 4}, {"ROL", ROL, zero_offset_x, 6}, {"XXX", NULL, NULL, 2}, {"SEC", SEC, implied, 2}, {"AND", AND, abs_offset_y, 4}, {"XXX", NULL, NULL, 2},       {"XXX", NULL, NULL, 2}, {"XXX", NULL, NULL, 2},        {"AND", AND, abs_offset_x, 4}, {"ROL", ROL, abs_offset_x, 7}, {"XXX", NULL, NULL, 2}, // 3-
	{"RTI", RTI, implied, 6},   {"EOR", EOR, zero_indirect_x, 6}, {"XXX", NULL, NULL, 2},     {"XXX", NULL, NULL, 2}, {"XXX", NULL, NULL, 2},         {"EOR", EOR, zero_page, 3},     {"LSR", LSR, zero_page, 5},     {"XXX", NULL, NULL, 2}, {"PHA", PHA, implied, 3}, {"EOR", EOR, immediate, 2},    {"LSR", LSR, accumulator, 2}, {"XXX", NULL, NULL, 2}, {"JMP", JMP, absolute, 3},     {"EOR", EOR, absolute, 4},     {"LSR", LSR, absolute, 6},     {"XXX", NULL, NULL, 2}, // 4-
	{"BVC", BVC, relative, 2},  {"EOR", EOR, zero_indirect_y, 5}, {"XXX", NULL, NULL, 2},     {"XXX", NULL, NULL, 2}, {"XXX", NULL, NULL, 2},         {"EOR", EOR, zero_offset_x, 4}, {"LSR", LSR, zero_offset_x, 6}, {"XXX", NULL, NULL, 2}, {"CLI", CLI, implied, 2}, {"EOR", EOR, abs_offset_y, 4}, {"XXX", NULL, NULL, 2},       {"XXX", NULL, NULL, 2}, {"XXX", NULL, NULL, 2},        {"EOR", EOR, abs_offset_x, 4}, {"LSR", LSR, abs_offset_x, 7}, {"XXX", NULL, NULL, 2}, // 5-
	{"RTS", RTS, implied, 6},   {"ADC", ADC, zero_indirect_x, 6}, {"XXX", NULL, NULL, 2},     {"XXX", NULL, NULL, 2}, {"XXX", NULL, NULL, 2},         {"ADC", ADC, zero_page, 3},     {"ROR", ROR, zero_page, 5},     {"XXX", NULL, NULL, 2}, {"PLA", PLA, implied, 4}, {"ADC", ADC, immediate, 2},    {"ROR", ROR, accumulator, 2}, {"XXX", NULL, NULL, 2}, {"JMP", JMP, indirect, 5},     {"ADC", ADC, absolute, 4},     {"ROR", ROR, absolute, 6},     {"XXX", NULL, NULL, 2}, // 6-
	{"BVS", BVS, relative, 2},  {"ADC", ADC, zero_indirect_y, 5}, {"XXX", NULL, NULL, 2},     {"XXX", NULL, NULL, 2}, {"XXX", NULL, NULL, 2},         {"ADC", ADC, zero_offset_x, 4}, {"ROR", ROR, zero_offset_x, 6}, {"XXX", NULL, NULL, 2}, {"SEI", SEI, implied, 2}, {"ADC", ADC, abs_offset_y, 4}, {"XXX", NULL, NULL, 2},       {"XXX", NULL, NULL, 2}, {"XXX", NULL, NULL, 2},        {"ADC", ADC, abs_offset_x, 4}, {"ROR", ROR, abs_offset_x, 7}, {"XXX", NULL, NULL, 2}, // 7-
	{"XXX", NULL, NULL, 2},     {"STA", STA, zero_indirect_x, 6}, {"XXX", NULL, NULL, 2},     {"XXX", NULL, NULL, 2}, {"STY", STY, zero_page, 3},     {"STA", STA, zero_page, 3},     {"STX", STX, zero_page, 3},     {"XXX", NULL, NULL, 2}, {"DEY", DEY, implied, 2}, {"XXX", NULL, NULL, 2},        {"TXA", TXA, implied, 2},     {"XXX", NULL, NULL, 2}, {"STY", STY, absolute, 4},     {"STA", STA, absolute, 4},     {"STX", STX, absolute, 4},     {"XXX", NULL, NULL, 2}, // 8-
	{"BCC", BCC, relative, 2},  {"STA", STA, zero_indirect_y, 6}, {"XXX", NULL, NULL, 2},     {"XXX", NULL, NULL, 2}, {"STY", STY, zero_offset_x, 4}, {"STA", STA, zero_offset_x, 4}, {"STX", STX, zero_offset_y, 4}, {"XXX", NULL, NULL, 2}, {"TYA", TYA, implied, 2}, {"STA", STA, abs_offset_y, 5}, {"TXS", TXS, implied, 2},     {"XXX", NULL, NULL, 2}, {"XXX", NULL, NULL, 2},        {"STA", STA, abs_offset_x, 5}, {"XXX", NULL, NULL, 2},        {"XXX", NULL, NULL, 2}, // 9-
	{"LDY", LDY, immediate, 2}, {"LDA", LDA, zero_indirect_x, 6}, {"LDX", LDX, immediate, 2}, {"XXX", NULL, NULL, 2}, {"LDY", LDY, zero_page, 3},     {"LDA", LDA, zero_page, 3},     {"LDX", LDX, zero_page, 3},     {"XXX", NULL, NULL, 2}, {"TAY", TAY, implied, 2}, {"LDA", LDA, immediate, 2},    {"TAX", TAX, implied, 2},     {"XXX", NULL, NULL, 2}, {"LDY", LDY, absolute, 4},     {"LDA", LDA, absolute, 4},     {"LDX", LDX, absolute, 4},     {"XXX", NULL, NULL, 2}, // A-
	{"BCS", BCS, relative, 2},  {"LDA", LDA, zero_indirect_y, 5}, {"XXX", NULL, NULL, 2},     {"XXX", NULL, NULL, 2}, {"LDY", LDY, zero_offset_x, 4}, {"LDA", LDA, zero_offset_x, 4}, {"LDX", LDX, zero_offset_y, 4}, {"XXX", NULL, NULL, 2}, {"CLV", CLV, implied, 2}, {"LDA", LDA, abs_offset_y, 4}, {"TSX", TSX, implied, 2},     {"XXX", NULL, NULL, 2}, {"LDY", LDY, abs_offset_x, 4}, {"LDA", LDA, abs_offset_x, 4}, {"LDX", LDX, abs_offset_y, 4}, {"XXX", NULL, NULL, 2}, // B-
	{"CPY", CPY, immediate, 2}, {"CMP", CMP, zero_indirect_x, 6}, {"XXX", NULL, NULL, 2},     {"XXX", NULL, NULL, 2}, {"CPY", CPY, zero_page, 3},     {"CMP", CMP, zero_page, 3},     {"DEC", DEC, zero_page, 5},     {"XXX", NULL, NULL, 2}, {"INY", INY, implied, 2}, {"CMP", CMP, immediate, 2},    {"DEX", DEX, implied, 2},     {"XXX", NULL, NULL, 2}, {"CPY", CPY, absolute, 4},     {"CMP", CMP, absolute, 4},     {"DEC", DEC, absolute, 6},     {"XXX", NULL, NULL, 2}, // C-
	{"BNE", BNE, relative, 2},  {"CMP", CMP, zero_indirect_y, 5}, {"XXX", NULL, NULL, 2},     {"XXX", NULL, NULL, 2}, {"XXX", NULL, NULL, 2},         {"CMP", CMP, zero_offset_x, 4}, {"DEC", DEC, zero_offset_x, 6}, {"XXX", NULL, NULL, 2}, {"CLD", CLD, implied, 2}, {"CMP", CMP, abs_offset_y, 4}, {"XXX", NULL, NULL, 2},       {"XXX", NULL, NULL, 2}, {"XXX", NULL, NULL, 2},        {"CMP", CMP, abs_offset_x, 4}, {"DEC", DEC, abs_offset_x, 7}, {"XXX", NULL, NULL, 2}, // D-
	{"CPX", CPX, immediate, 2}, {"SBC", SBC, zero_indirect_x, 6}, {"XXX", NULL, NULL, 2},     {"XXX", NULL, NULL, 2}, {"CPX", CPX, zero_page, 3},     {"SBC", SBC, zero_page, 3},     {"INC", INC, zero_page, 5},     {"XXX", NULL, NULL, 2}, {"INX", INX, implied, 2}, {"SBC", SBC, immediate, 2},    {"NOP", NOP, implied, 2},     {"XXX", NULL, NULL, 2}, {"CPX", CPX, absolute, 4},     {"SBC", SBC, absolute, 4},     {"INC", INC, absolute, 6},     {"XXX", NULL, NULL, 2}, // E-
	{"BEQ", BEQ, relative, 2},  {"SBC", SBC, zero_indirect_y, 5}, {"XXX", NULL, NULL, 2},     {"XXX", NULL, NULL, 2}, {"XXX", NULL, NULL, 2},         {"SBC", SBC, zero_offset_x, 4}, {"INC", INC, zero_offset_x, 6}, {"XXX", NULL, NULL, 2}, {"SED", SED, implied, 2}, {"SBC", SBC, abs_offset_y, 4}, {"XXX", NULL, NULL, 2},       {"XXX", NULL, NULL, 2}, {"XXX", NULL, NULL, 2},        {"SBC", SBC, abs_offset_x, 4}, {"INC", INC, abs_offset_x, 7}, {"XXX", NULL, NULL, 2}, // F-
};

typedef struct NES NES;
void cpu_write(NES *, uint16_t, uint8_t);
uint8_t cpu_read(NES *, uint16_t);

CPU *init_cpu()
{
	CPU *cpu = malloc(sizeof(CPU));
	reset_cpu(cpu);
	return cpu;
}

void run_next_instruction(CPU *cpu)
{
	cpu->operand = 0x0000;

	uint8_t opcode = cpu_read(cpu, cpu->PC);
	Instruction *current_inst = &instruction_table[opcode];
	cpu->current_inst = current_inst;

	cpu->current_cycles += current_inst->clock_cycles;
	cpu->total_cycles   += current_inst->clock_cycles;

	fprintf(stdout, "%04X:  ", cpu->PC);

	current_inst->addr_mode(cpu);
	current_inst->operation(cpu);
}

void connect_system(CPU *cpu, struct NES *nes) { cpu->nes = nes; }

void reset_cpu(CPU *cpu)
{
	memset(cpu, 0, sizeof(CPU));
	cpu->U = 1;  // unused flag bit 5 is always 1
	cpu->I = 1;

	uint8_t little, big;
	little = cpu->memory[RESET_LO];
	big = cpu->memory[RESET_HI];
	cpu->PC = (uint16_t)big << 8 | little;

	cpu->SP = STK_PTR_START;
	cpu->total_cycles = CPU_CLK_START;
}

void delete_cpu(CPU *cpu)
{
	free(cpu);
}

uint8_t get_flags(CPU *cpu)
{
	uint8_t result = 0x00;

	result |= cpu->C << 0;
	result |= cpu->Z << 1;
	result |= cpu->I << 2;
	result |= cpu->D << 3;
	result |= cpu->B << 4;
	result |= cpu->U << 5;
	result |= cpu->V << 6;
	result |= cpu->N << 7;

	return result;
}

void set_flags(CPU *cpu, uint8_t flags)
{
	cpu->C = flags & 1 << 0 ? 1 : 0;
	cpu->Z = flags & 1 << 1 ? 1 : 0;
	cpu->I = flags & 1 << 2 ? 1 : 0;
	cpu->D = flags & 1 << 3 ? 1 : 0;
	cpu->B = flags & 1 << 4 ? 1 : 0;
	cpu->U = 1;  // always 1
	cpu->V = flags & 1 << 6 ? 1 : 0;
	cpu->N = flags & 1 << 7 ? 1 : 0;
}

void dump_cpu(CPU *cpu, FILE *f)
{
	for (size_t i = 0; i < 256; i++)
	{
		if (i % 8 == 0)
			printf("\n");
		printf("%02X ", cpu->memory[i]);
	}

	fprintf(f, "\nA:%02X X:%02X Y:%02X P:%02X SP:%02X  PPU: --, -- CYC:%u\n\n", cpu->A, cpu->X, cpu->Y, get_flags(cpu), cpu->SP, cpu->total_cycles);
	fprintf(f, "Flags: NVUBDIZC\n       %d%d%d%d%d%d%d%d\n\n", cpu->N, cpu->V, cpu->U, cpu->B, cpu->D, cpu->I, cpu->Z, cpu->C);
}

void inc_stack_ptr(CPU *cpu)
{
	if (cpu->SP == 0xFF)
	{
		fprintf(stderr, "[ERROR] Stack underflow; exiting...");
		exit(EXIT_FAILURE);
	}
	cpu->SP++;
}

void dec_stack_ptr(CPU *cpu)
{
	if (cpu->SP == 0x00)
	{
		fprintf(stderr, "[ERROR] Stack overflow; exiting...");
		exit(EXIT_FAILURE);
	}
	cpu->SP--;
}

void stack_push(CPU *cpu, uint8_t value)
{
	cpu_write(cpu->nes, STACK_START + cpu->SP, value);
	dec_stack_ptr(cpu);
}

uint8_t stack_pop(CPU *cpu)
{
	inc_stack_ptr(cpu);
	return cpu_read(cpu->nes, STACK_START + cpu->SP);
}

void stack_push_word(CPU *cpu, uint16_t word)
{
	stack_push(cpu, (word >> 8) & 0x00FF); 
	stack_push(cpu, word & 0x00FF);
}

uint16_t stack_pop_word(CPU *cpu)
{
	uint16_t result = 0x0000;

	result |= stack_pop(cpu);
	result |= (uint16_t)stack_pop(cpu) << 8;

	return result;
}

// This will read the whole file into an array of bytes...
// maybe there is a smarter buffered way to do this
uint8_t *read_file_as_bytes(char *file_name, size_t *file_len)
{
	FILE *f = fopen(file_name, "rb");
	if (f == NULL)
	{
		perror("fopen");
		exit(1);
	}

	fseek(f, 0, SEEK_END);
	*file_len = (size_t)ftell(f);
	rewind(f);

	uint8_t *buffer = malloc(*file_len);
	fread(buffer, 1, *file_len, f);
	fclose(f);

	return buffer;
}


// 6502 assembler: https://www.masswerk.at/6502/assembler.html
void run_program(CPU *cpu, FILE *logfile)
{
	uint8_t opcode;
	Instruction *current_inst;
	dump_cpu(cpu, stdout);

	fprintf(logfile, "\n");
	for (size_t inst_count = 0; cpu->PC < 0xFFFF; ++inst_count) 
	{
		fprintf(logfile, "-----------RESULT OF INST %lu-----------\n\n", inst_count);
		fprintf(logfile, "%04X: ", cpu->PC);

		cpu->operand = 0x0000;

		opcode = cpu->memory[cpu->PC];
		current_inst = &instruction_table[opcode];
		cpu->current_inst = current_inst;

		cpu->current_cycles += current_inst->clock_cycles;
		cpu->total_cycles   += current_inst->clock_cycles;

		current_inst->addr_mode(cpu);
		current_inst->operation(cpu);

		

		if (inst_count > 3363)
		{
			dump_cpu(cpu, stdout);
			getchar();
		}

		// TODO - implement clock
		while (cpu->current_cycles)
			cpu->current_cycles--;

	}
	
}

/* 
ADDRESSING MODES
see: https://rosettacode.org/wiki/Category:6502_Assembly#Addressing_Modes
more: http://www.emulator101.com/6502-addressing-modes.html
*/

// Implied instructions have no operands
void implied(CPU *cpu)
{
	cpu->PC += 1;
	
	fprintf(assembly_outfile, "%s\n", cpu->current_inst->name);
}

// Operand is accumulator
void accumulator(CPU *cpu)
{
	
	cpu->operand = cpu->A;
	cpu->PC += 1;

	fprintf(assembly_outfile, "%s A\n", cpu->current_inst->name);
}

// The operand of an immediate instruction is only one byte, and denotes a constant value
void immediate(CPU *cpu)
{
	cpu->operand = cpu_read(cpu->nes, cpu->PC + 1);
	cpu->jmp_addr = cpu->operand;
	cpu->PC += 2;

	fprintf(assembly_outfile, "%s #$%02X\n", cpu->current_inst->name, cpu->operand);
}

// The operand of a zeropage instruction is one byte, and denotes an address in the zero page
void zero_page(CPU *cpu)
{
	uint8_t value = cpu_read(cpu->nes, cpu->PC + 1);
	fprintf(assembly_outfile, "%s $%02X\n", cpu->current_inst->name, value);

	cpu->jmp_addr = (uint16_t)value & 0x00FF;
	cpu->operand = cpu_read(cpu->nes, value);
	cpu->PC += 2;
}

// The operand of an absolute instruction is two bytes, and denotes an address in memory
void absolute(CPU *cpu)
{
	uint8_t little, big;
	little = cpu_read(cpu->nes, cpu->PC + 1);
	big = cpu_read(cpu->nes, cpu->PC + 2);
	uint16_t addr = (uint16_t)big << 8 | little;

	cpu->jmp_addr = addr;

	cpu->operand = cpu_read(cpu->nes, addr);
	cpu->PC += 3;

	fprintf(assembly_outfile, "%s $%02X%02X\n", cpu->current_inst->name, big, little);
}

// Indirect: operand is address; effective address is contents of word at address
// only used by JMP
void indirect(CPU *cpu)
{
	uint8_t little, big;
	little = cpu_read(cpu->nes, cpu->PC + 1);
	big = cpu_read(cpu->nes, cpu->PC + 2);
	uint16_t addr = (uint16_t)big << 8 | little;

	fprintf(assembly_outfile, "%s ($%02X%02X)\n", cpu->current_inst->name, big, little);
	if (little == 0xFF)
		big = cpu_read(cpu->nes, addr - 0xFF); // no carry bug
	else  
		big = cpu_read(cpu->nes, addr + 1);
	little = cpu_read(cpu->nes, addr);

	cpu->jmp_addr = (uint16_t)big << 8 | little;
	cpu->operand = cpu_read(cpu->nes, cpu->jmp_addr);
	cpu->PC += 3;
}

// Set the operand to PC + the *signed* byte in the next 
void relative(CPU *cpu)
{
	uint8_t offset = cpu_read(cpu->nes, cpu->PC + 1);

	cpu->operand = offset;
	// cpu->jmp_addr = cpu->PC + (int8_t)offset;
	// printf("%u = %u + %d   %u  %d\n", cpu->jmp_addr, cpu->PC, (int8_t)offset, offset, offset);
	cpu->PC += 2;
	fprintf(assembly_outfile, "%s $%04X\n", cpu->current_inst->name, cpu->PC + (int8_t)offset);
}

// A zero page memory address offset by X
void zero_offset_x(CPU *cpu)
{
	fprintf(assembly_outfile, "%s $%02X,X\n", cpu->current_inst->name, cpu_read(cpu->nes, cpu->PC + 1));

	uint8_t index = (cpu_read(cpu->nes, cpu->PC + 1) + cpu->X) % 256;
	cpu->jmp_addr = (uint16_t)index & 0x00FF;	
	cpu->operand = cpu_read(cpu->nes, index);
	cpu->PC += 2;
}

// A zero page memory address offset by Y
void zero_offset_y(CPU *cpu)
{
	fprintf(assembly_outfile, "%s $%02X,Y\n", cpu->current_inst->name, cpu_read(cpu->nes, cpu->PC + 1));

	uint8_t index = (cpu_read(cpu->nes, cpu->PC + 1) + cpu->Y) % 256;

	cpu->jmp_addr = (uint16_t)index;

	cpu->operand = cpu_read(cpu->nes, index);
	cpu->PC += 2;
}

// An absolute memory address offset by X
void abs_offset_x(CPU *cpu)
{
	uint8_t little, big;
	little = cpu_read(cpu->nes, cpu->PC + 1);
	big = cpu_read(cpu->nes, cpu->PC + 2);
	uint16_t addr = (uint16_t)big << 8 | little;

	fprintf(assembly_outfile, "%s $%02X%02X,X\n", cpu->current_inst->name, little, big);

	cpu->jmp_addr = addr + (uint16_t)cpu->X;
	cpu->operand = cpu_read(cpu->nes, cpu->jmp_addr);
	cpu->PC += 3;
}

// An absolute memory address offset by Y
void abs_offset_y(CPU *cpu)
{
	uint8_t little, big;
	little = cpu_read(cpu->nes, cpu->PC + 1);
	big = cpu_read(cpu->nes, cpu->PC + 2);
	uint16_t addr = (uint16_t)big << 8 | little;

	fprintf(assembly_outfile, "%s $%02X%02X,Y\n", cpu->current_inst->name, little, big);

	cpu->jmp_addr = addr + (uint16_t)cpu->Y;
	cpu->operand = cpu_read(cpu->nes, cpu->jmp_addr);
	cpu->PC += 3;
}

void zero_indirect_x(CPU *cpu)
{

	uint8_t little, big, addr;
	addr = cpu_read(cpu->nes, cpu->PC + 1) + cpu->X;
	little = cpu_read(cpu->nes, addr);
	// "Increments without carry do not affect the hi-byte of an address and no page transitions do occur"
	if (addr == 0xFF)
		big = cpu_read(cpu->nes, 0x00);
	else
		big = cpu_read(cpu->nes, addr + 1);
	uint16_t final_addr = (uint16_t)big << 8 | little;

	fprintf(assembly_outfile, "%s ($%02X,X)\n", cpu->current_inst->name, cpu_read(cpu->nes, cpu->PC + 1));

	cpu->jmp_addr = final_addr;

	cpu->operand = cpu_read(cpu->nes, final_addr);
	cpu->PC += 2;
}

void zero_indirect_y(CPU *cpu)
{
	uint8_t little, big, val;
	val = cpu_read(cpu->nes, cpu->PC + 1);
	little = cpu_read(cpu->nes, val);
	if (val == 0xFF)
		big = cpu_read(cpu->nes, 0x00);
	else
		big = cpu_read(cpu->nes, val + 1);
	
	uint16_t addr = (uint16_t)big << 8 | little;


	fprintf(assembly_outfile, "%s ($%02X),Y\n", cpu->current_inst->name, cpu_read(cpu->nes, cpu->PC + 1));	

	cpu->jmp_addr = addr + ((uint16_t)cpu->Y & 0x00FF);
	printf("%04X %04X\n", addr, cpu->jmp_addr);

	cpu->operand = cpu_read(cpu->nes, cpu->jmp_addr);
	cpu->PC += 2;
}


// INSTRUCTIONS
// details: https://llx.com/Neil/a2/opcodes.html
// more: http://www.emulator101.com/reference/6502-reference.html

static uint8_t check_negative(uint8_t value)
{
	return value & 0x80 ? 1 : 0;
}

static uint8_t check_zero(uint8_t value)
{
	return value ? 0 : 1;
}

static uint8_t check_carry(uint8_t value)
{
	return value & 0x80 ? 1 : 0;
}

// group 1
void ORA(CPU *cpu)
{
	cpu->A |= cpu->operand;
	cpu->N = check_negative(cpu->A);
	cpu->Z = check_zero(cpu->A);
}

void AND(CPU *cpu)
{
	cpu->A &= cpu->operand;
	cpu->N = check_negative(cpu->A);
	cpu->Z = check_zero(cpu->A);
}

void EOR(CPU *cpu)
{
	cpu->A ^= cpu->operand;
	cpu->N = check_negative(cpu->A);
	cpu->Z = check_zero(cpu->A);
}


// See details for complicated ADC flag settings here:
// https://github.com/OneLoneCoder/olcNES/blob/master/Part%232%20-%20CPU/olc6502.cpp#L597
void ADC(CPU *cpu)
{
	uint16_t temp = (uint16_t)cpu->A + (uint16_t)cpu->operand + (uint16_t)cpu->C;
	cpu->C = temp > 255 ? 1 : 0;
	cpu->Z = (temp & 0x00FF) == 0 ? 1 : 0;

	/*
	The over flow is set if the two operands of the addition have the same sign, and the result has the opposite sign
	so if the MSB of both operands is 1 and the MSB of the result (temp) is 0, or vice versa,
	then the overflow flag is set. Notice that the final '&' here will result in 
	0x80 if this is true on the left-hand side and it will be 0x00 otherwise.
	*/
	cpu->V = (~((uint16_t)cpu->A ^ (uint16_t)cpu->operand) & ((uint16_t)cpu->A ^ (uint16_t)temp)) & 0x0080 ? 1 : 0;
	cpu->N = temp & 0x80 ? 1 : 0;

	cpu->A = (uint8_t)(temp & 0x00FF);
}

void STA(CPU *cpu)
{
	cpu_write(cpu->nes, cpu->jmp_addr, cpu->A);
}

void LDA(CPU *cpu)
{
	cpu->A = cpu->operand;
	cpu->Z = check_zero(cpu->A);
	cpu->N = check_negative(cpu->A);
}

void CMP(CPU *cpu)
{
	uint16_t temp = (uint16_t)cpu->A - (uint16_t)cpu->operand;
	cpu->N = temp & 0x0080 ? 1 : 0;
	cpu->Z = check_zero(temp & 0x00FF);
	cpu->C = (cpu->A >= cpu->operand) ? 1 : 0;
}

void SBC(CPU *cpu)
{
	// invert the operand bits, and SBC becomes the same as ADC (i.e. ADC(x) == SBC(~x))
	cpu->operand ^= 0x00FF;

	// TODO - determine if this is sufficient
	ADC(cpu);
}


// group 2
void ASL(CPU *cpu)
{
	uint8_t temp = cpu->operand << 1;
	if (cpu->current_inst->addr_mode == accumulator)
		cpu->A = temp;
	else
		cpu_write(cpu->nes, cpu->jmp_addr, temp);
	cpu->C = check_carry(cpu->operand);
	cpu->N = check_negative(temp);
	cpu->Z = check_zero(temp);
}

void ROL(CPU *cpu)
{

	uint8_t temp = (cpu->operand << 1) + cpu->C;
	cpu->C = check_carry(cpu->operand);
	if (cpu->current_inst->addr_mode == accumulator)
		cpu->A = temp;
	else
		cpu_write(cpu->nes, cpu->jmp_addr, temp);

	cpu->N = check_negative(temp);
	cpu->Z = check_zero(temp);
}	

void LSR(CPU *cpu)
{
	uint8_t temp = cpu->operand >> 1;
	if (cpu->current_inst->addr_mode == accumulator)
		cpu->A = temp;
	else
		cpu_write(cpu->nes, cpu->jmp_addr, temp);
	cpu->C = cpu->operand & 0x01 ? 1 : 0;
	cpu->N = 0;
	cpu->Z = check_zero(temp);
}

void ROR(CPU *cpu)
{

	uint8_t temp = (cpu->operand >> 1) | (cpu->C << 7);

	cpu->C = cpu->operand & 0x01 ? 1 : 0;

	if (cpu->current_inst->addr_mode == accumulator)
		cpu->A = temp;
	else
		cpu_write(cpu->nes, cpu->jmp_addr, temp);

	cpu->N = check_negative(temp);
	cpu->Z = check_zero(temp);
}

void STX(CPU *cpu)
{
		cpu_write(cpu->nes, cpu->jmp_addr, cpu->X);
}

void LDX(CPU *cpu)
{
	cpu->X = cpu->operand;
	cpu->Z = check_zero(cpu->X);
	cpu->N = check_negative(cpu->X);
}

void INC(CPU *cpu)
{
	cpu->operand++;
	cpu_write(cpu->nes, cpu->jmp_addr, cpu->operand);
	cpu->Z = check_zero(cpu->operand);
	cpu->N = check_negative(cpu->operand);
}

void DEC(CPU *cpu)
{
	cpu->operand--;
	cpu_write(cpu->nes, cpu->jmp_addr, cpu->operand);
	cpu->Z = check_zero(cpu->operand);
	cpu->N = check_negative(cpu->operand);
}


// group 3
void BIT(CPU *cpu)
{
	printf("%02X %02X\n", cpu->operand, cpu->A);
	cpu->Z = check_zero((cpu->operand & cpu->A) & 0x00FF);
	cpu->V = cpu->operand & (1 << 6) ? 1 : 0;
	cpu->N = cpu->operand & (1 << 7) ? 1 : 0;
}

void JMP(CPU *cpu)
{
	cpu->PC = cpu->jmp_addr;
}

void STY(CPU *cpu)
{
	cpu_write(cpu->nes, cpu->jmp_addr, cpu->Y);
}

void LDY(CPU *cpu)
{
	cpu->Y = cpu->operand;
	cpu->Z = check_zero(cpu->Y);
	cpu->N = check_negative(cpu->Y);
}

void CPY(CPU *cpu)
{
	uint16_t temp = (uint16_t)cpu->Y - (uint16_t)cpu->operand;
	cpu->N = temp & 0x0080 ? 1 : 0;
	cpu->Z = check_zero(temp & 0x00FF);
	cpu->C = (cpu->Y >= cpu->operand) ? 1 : 0;
}

void CPX(CPU *cpu)
{
	uint16_t temp = (uint16_t)cpu->X - (uint16_t)cpu->operand;
	cpu->N = temp & 0x0080 ? 1 : 0;
	cpu->Z = check_zero(temp & 0x00FF);
	cpu->C = (cpu->X >= cpu->operand) ? 1 : 0;
}


// conditional branches
void BPL(CPU *cpu)
{
	if (!cpu->N)
		cpu->PC += (int8_t)cpu->operand;
}

void BMI(CPU *cpu)
{
	if (cpu->N)
		cpu->PC += (int8_t)cpu->operand;
}

void BVC(CPU *cpu)
{
	if (!cpu->V)
		cpu->PC += (int8_t)cpu->operand;
}

void BVS(CPU *cpu)
{
	if (cpu->V)
		cpu->PC += (int8_t)cpu->operand;
}

void BCC(CPU *cpu)
{
	if (!cpu->C)
		cpu->PC += (int8_t)cpu->operand;
}

void BCS(CPU *cpu)
{
	if (cpu->C)
		cpu->PC += (int8_t)cpu->operand;
}

void BNE(CPU *cpu)
{
	if (!cpu->Z)
		cpu->PC += (int8_t)cpu->operand;
}

void BEQ(CPU *cpu)
{
	if (cpu->Z)
		cpu->PC += (int8_t)cpu->operand;
}


// other
void BRK(CPU *cpu)
{
	uint8_t flags = get_flags(cpu);
	flags |= 1 << 4;

	stack_push_word(cpu, cpu->PC + 1);
	stack_push(cpu, flags);

	cpu->B = 0;

	cpu->PC = ((uint16_t)cpu->memory[IRQ_LO]) | ((uint16_t)cpu->memory[IRQ_HI] << 8);
}

void JSR(CPU *cpu)
{
	cpu->PC -= 1;

	stack_push_word(cpu, cpu->PC);

	cpu->PC = cpu->jmp_addr;
}

void RTI(CPU *cpu)
{
	uint8_t B = cpu->B;
	set_flags(cpu, stack_pop(cpu));
	cpu->B = B;

	cpu->PC = stack_pop_word(cpu);
}

void RTS(CPU *cpu)
{
	cpu->PC = stack_pop_word(cpu);
	cpu->PC++;
}

void PHP(CPU *cpu)
{
	uint8_t flags = get_flags(cpu);
	flags |= 1 << 4;
	flags |= 1 << 5;
	stack_push(cpu, flags);
}

void PLP(CPU *cpu)
{
	uint8_t B = cpu->B;
	set_flags(cpu, stack_pop(cpu));
	cpu->B = B;
}

void PHA(CPU *cpu)
{
	stack_push(cpu, cpu->A);
}

void PLA(CPU *cpu)
{
	cpu->A = stack_pop(cpu);
	cpu->Z = check_zero(cpu->A);
	cpu->N = check_negative(cpu->A);
}

void DEY(CPU *cpu)
{
	cpu->Y--;
	cpu->Z = check_zero(cpu->Y);
	cpu->N = check_negative(cpu->Y);
}

void TAY(CPU *cpu)
{
	cpu->Y = cpu->A;
	cpu->Z = check_zero(cpu->Y);
	cpu->N = check_negative(cpu->Y);
}

void INY(CPU *cpu)
{
	cpu->Y++;
	cpu->Z = check_zero(cpu->Y);
	cpu->N = check_negative(cpu->Y);
}

void INX(CPU *cpu)
{
	cpu->X++;
	cpu->Z = check_zero(cpu->X);
	cpu->N = check_negative(cpu->X);
}	

void CLC(CPU *cpu)
{
	cpu->C = 0;
}

void SEC(CPU *cpu)
{
	cpu->C = 1;
}

void CLI(CPU *cpu)
{
	cpu->I = 0;
}

void SEI(CPU *cpu)
{
	cpu->I = 1;
}

void TYA(CPU *cpu)
{
	cpu->A = cpu->Y;
	cpu->Z = check_zero(cpu->A);
	cpu->N = check_negative(cpu->A);
}

void CLV(CPU *cpu)
{
	cpu->V = 0;
}

void CLD(CPU *cpu)
{
	cpu->D = 0;
}

void SED(CPU *cpu)
{
	cpu->D = 1;
}

void TXA(CPU *cpu)
{
	cpu->A = cpu->X;
	cpu->Z = check_zero(cpu->X);
	cpu->N = check_negative(cpu->X);
}

void TXS(CPU *cpu)
{
	cpu->SP = cpu->X;
}

void TAX(CPU *cpu)
{
	cpu->X = cpu->A;
	cpu->Z = check_zero(cpu->X);
	cpu->N = check_negative(cpu->X);
}

void TSX(CPU *cpu)
{
	cpu->X = cpu->SP;
	cpu->Z = check_zero(cpu->X);
	cpu->N = check_negative(cpu->X);
}

void DEX(CPU *cpu)
{
	cpu->X--;
	cpu->Z = check_zero(cpu->X);
	cpu->N = check_negative(cpu->X);
}

void NOP(CPU *cpu)
{
	(void) cpu;
}


// Interrupts

void IMP(CPU *cpu)
{
	if (cpu->I == 0)
	{
		stack_push_word(cpu, cpu->PC);

		cpu->B = 0;
		cpu->U = 1;
		cpu->I = 1;
		stack_push(cpu, get_flags(cpu));

		uint16_t little = cpu->memory[IRQ_LO];
		uint8_t  big    = cpu->memory[IRQ_HI];
		cpu->PC = little | (big << 8);

		cpu->current_cycles += 7;
		cpu->total_cycles   += 7;
	}

}

void NMI(CPU *cpu)
{
		stack_push_word(cpu, cpu->PC);

		cpu->B = 0;
		cpu->U = 1;
		cpu->I = 1;
		stack_push(cpu, get_flags(cpu));

		uint16_t little = cpu->memory[NMI_LO];
		uint8_t  big    = cpu->memory[NMI_HI];
		cpu->PC = little | (big << 8);

		cpu->current_cycles += 8;
		cpu->total_cycles   += 8;
}
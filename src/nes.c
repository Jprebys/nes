#include <stdlib.h>

#include "nes.h"
#include "cpu.h"
#include "cart.h"

#define VRAM_MAX_ADDR    0x2000
#define PPU_REG_MAX_ADDR 0x4000


NES *init_nes()
{
	NES *nes = malloc(sizeof(NES));
	nes->cpu = init_cpu();
	nes->ppu = init_ppu();
	nes->cart = NULL;
	return nes;
}

void delete_nes(NES *nes)
{
	if (nes->cart != NULL)
		delete_cart(nes->cart);
	free(nes->ppu);
	free(nes->cpu);
	free(nes);
}

void dump_nes_info(NES *nes, char *buffer)
{
	CPU *cpu = nes->cpu;
	char *pos = buffer;
	for (size_t i = 0; i < 256; i++)
	{
		if (i % 8 == 0)
			pos += sprintf(pos, "\n");
		pos += sprintf(pos, "%02X ", cpu->memory[i]);

	}

	pos += sprintf(pos, "\n\nA:%02X X:%02X Y:%02X P:%02X\nSP:%02X PPU: --, -- CYC:%u\n\n", cpu->A, cpu->X, cpu->Y, get_flags(cpu), cpu->SP, cpu->total_cycles);
	pos += sprintf(pos, "Flags: NVUBDIZC\n       %d%d%d%d%d%d%d%d\n\n", cpu->N, cpu->V, cpu->U, cpu->B, cpu->D, cpu->I, cpu->Z, cpu->C);
	*pos = '\0';

}


void mem_write(NES *nes, uint16_t addr, uint8_t value)
{
	if (addr < VRAM_MAX_ADDR)
	{
		// CPU VRAM is mirrored 4 ways so we strip
		// off the 2 most significant bits
		addr &= 0b0011111111111;
		nes->cpu->memory[addr] = value;
	} 
	else if (addr < PPU_REG_MAX_ADDR)
	{
		// PPU registers are addr 0x2000 through 0x2007
		// and they are mirrored up to 0x3FFF
		addr &= 0b0010000000000111;
		switch (addr) {
			case 0x2000:
				set_ppuctrl(nes->ppu, value);
				break;
			case 0x2001:
				set_ppumask(nes->ppu, value);
				break;
			case 0x2002:
				set_ppustatus(nes->ppu, value);
				break;
			case 0x2003:
				nes->ppu->oam_addr = value;
				break;
			case 0x2004:
				nes->ppu->oam_data = value;
				break;
			case 0x2005:
				nes->ppu->scroll = value;
				break;
			case 0x2006:
				nes->ppu->addr = value;	
				break;
			case 0x2007:
				nes->ppu->data = value;
				break;
			default:
				perror("Invalid PPU Addr");
				exit(EXIT_FAILURE);
		}
	} else {
		printf("Ignoring write to address %04X\n", addr);
	}
}

uint8_t mem_read(NES *nes, uint16_t addr) {
	if (addr < VRAM_MAX_ADDR) {
		// CPU VRAM is mirrored 4 ways so we strip
		// off the 2 most significant bits
		addr &= 0b0011111111111;
		return nes->cpu->memory[addr];
	} else if (addr < PPU_REG_MAX_ADDR) {
		// PPU registers are addr 0x2000 through 0x2007
		// and they are mirrored up to 0x3FFF
		addr &= 0b0010000000000111;
		switch (addr) {
			case 0x2000:
				return get_ppuctrl(nes->ppu);
			case 0x2001:
				return get_ppumask(nes->ppu);
			case 0x2002:
				return get_ppustatus(nes->ppu);
			case 0x2003:
				return nes->ppu->oam_addr;
			case 0x2004:
				return nes->ppu->oam_data;
			case 0x2005:
				return nes->ppu->scroll;
			case 0x2006:
				return nes->ppu->addr;	
			case 0x2007:
				return nes->ppu->data;
			default:
				perror("Invalid PPU Addr");
				exit(EXIT_FAILURE);
		}
	} else {
		printf("Ignoring read from address %04X; returning 0\n", addr);
		return 0x00;
	}

}




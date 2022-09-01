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
	connect_system(nes->cpu, nes);
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



void cpu_write(NES *nes, uint16_t addr, uint8_t value)
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
				printf("[WARNING] Attemting to write to read-only register PPU\n");
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
	} else if (addr == 0x4014) {
		// https://wiki.nesdev.com/w/index.php/PPU_programmer_reference#OAM_DMA_.28.244014.29_.3E_write
		oam_dma(nes, value);
	} else if (addr < 0x4016) {
		// TODO write to APU here
	} else if (addr == 0x4016) {
		printf("[WARNING] Attemting to write to controller 1; ignoring\n");
	} else if (addr == 0x4017) {
		printf("[WARNING] Attemting to write to controller 2; ignoring\n");
	} else if (addr < 0x6000) {
		printf("[WARNING] Attemting to write to expansion ROM; ignoring\n");
	} else if (addr < 0x8000) {
		// SRAM
		printf("[WARNING] Attemting to write to unimplemented SRAM; ignoring\n");
	} else {
		printf("[WARNING] Attemting to write to PRG ROM; ignoring\n");
	}
}


uint8_t cpu_read(NES *nes, uint16_t addr) {
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
				printf("[WARNING] Attemting to read write-only register PPUCTRL\n");
				return 0x00;
			case 0x2001:
				printf("[WARNING] Attemting to read write-only register PPUMASK\n");
				return 0x00;			
			case 0x2002:
				return get_ppustatus(nes->ppu);
			case 0x2003:
				printf("[WARNING] Attemting to read write-only register OAMADDR\n");
				return 0x00;
			case 0x2004:
				return nes->ppu->oam_data;
			case 0x2005:
				printf("[WARNING] Attemting to read write-only register PPUSCROLL\n");
				return 0x00;
			case 0x2006:
				printf("[WARNING] Attemting to read write-only register PPUADDR\n");
				return 0x00;
			case 0x2007:
				return nes->ppu->data;
			default:
				perror("Invalid PPU Addr");
				exit(EXIT_FAILURE);
		}
	} else if (addr < 0x4014) {
		printf("[WARNING] Attempting to read from write-only APU address %04X; returning 0\n", addr);
		return 0x00;
	} else if (addr == 0x4015) {
		// TODO - implement APU register
		return 0x00;
	} else if (addr == 0x4016) {
		return nes->controller1_state;
	} else if (addr == 0x4017) {
		return nes->controller2_state;
	} else if (addr < 0x6000) {
		printf("[WARNING] Attempting to read from unimplemented expansion ROM address %04X; returning 0\n", addr);
		return 0x00;
	} else if (addr < 0x8000) {
		printf("[WARNING] Attempting to read from unimplemented SRAM address %04X; returning 0\n", addr);
		return 0x00;
	} else {
		return cart_read_prg(nes->cart, addr);
	}

}

void oam_dma(NES *nes, uint8_t value)
{
	uint8_t data;
	uint8_t oam_start = nes->ppu->oam_addr;
	uint16_t cpu_mem_start = (uint16_t)value << 8;
	for (size_t i = 0; i < 256; ++i)
	{
		data = cpu_read(nes, cpu_mem_start + i);
		nes->ppu->oam_memory[(oam_start + i) % 256] = data;
	}
}

void ppu_read(NES *nes, uint16_t addr) {

}

uint8_t ppu_write(NES *nes, uint16_t addr, uint8_t value) {

}

void load_cartridge(NES *nes, Cartridge *cart) {
	nes->cart = cart;
}

void reset(NES *nes) {

}

void clock(NES *nes) {

}

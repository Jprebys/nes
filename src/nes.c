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
				nes->ppu->tram_addr.nametable_x = nes->ppu->ctrl.nametable_x;
				nes->ppu->tram_addr.nametable_y = nes->ppu->ctrl.nametable_y;
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
				// PPU Scroll
				if (!nes->ppu->address_latch) {
					nes->ppu->fine_x = value & 0x07;
					nes->ppu->tram_addr.coarse_x = value >> 3;
					nes->ppu->address_latch = true;
				} else {
					nes->ppu->tram_addr.fine_y = value & 0x07;
					nes->ppu->tram_addr.coarse_y = value >> 3;
					nes->ppu->address_latch = false;
				}
				break;
			case 0x2006:
				// PPU addr
				if (!nes->ppu->address_latch) {
					uint16_t data = (uint16_t)((value & 0x3F) << 8);
					data |= get_loopyregister(&nes->ppu->tram_addr) & 0x00FF;
					set_loopyregister(&nes->ppu->tram_addr, data);
					nes->ppu->address_latch = true;
				} else {
					uint16_t data = (get_loopyregister(&nes->ppu->tram_addr) & 0xFF00) | value;
					set_loopyregister(&nes->ppu->vram_addr, data);
					nes->ppu->address_latch = false;
				}
				break;
			case 0x2007:
				// PPU Data
				ppu_write(nes, get_loopyregister(&nes->ppu->vram_addr), value);

				uint16_t vram = get_loopyregister(&nes->ppu->vram_addr);
				uint8_t inc = nes->ppu->ctrl.increment_mode? 32 : 1;
				set_loopyregister(&nes->ppu->vram_addr, vram + inc);
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
	uint8_t data = 0x00;

	if (addr < VRAM_MAX_ADDR) {
		// CPU VRAM is mirrored 4 ways so we strip
		// off the 2 most significant bits
		addr &= 0b0011111111111;
		data = nes->cpu->memory[addr];
	} else if (addr < PPU_REG_MAX_ADDR) {
		// PPU registers are addr 0x2000 through 0x2007
		// and they are mirrored up to 0x3FFF
		addr &= 0b0010000000000111;
		switch (addr) {
			case 0x2000:
				printf("[WARNING] Attemting to read write-only register PPUCTRL\n");
				break;
			case 0x2001:	
				printf("[WARNING] Attemting to read write-only register PPUMASK\n");
				break;			
			case 0x2002:
				data = get_ppustatus(nes->ppu);

				// Possibly pick up noise from 5 small bits
				// left over from last PPU bus transaction
				data |= nes->ppu->data_buffer & 0x1F;

				nes->ppu->status.vertical_blank = 0;
				nes->ppu->address_latch = false;
				break;
			case 0x2003:
				printf("[WARNING] Attemting to read write-only register OAMADDR\n");
				break;
			case 0x2004:
				data = nes->ppu->oam_data;
				break;
			case 0x2005:
				printf("[WARNING] Attemting to read write-only register PPUSCROLL\n");
				break;
			case 0x2006:
				printf("[WARNING] Attemting to read write-only register PPUADDR\n");
				break;
			case 0x2007:
				data = nes->ppu->data_buffer;
				uint16_t vram_addr = get_loopyregister(&nes->ppu->vram_addr);
				nes->ppu->data_buffer = ppu_read(nes, vram_addr);
				
				if (vram_addr >= 0x3F00)
					data = nes->ppu->data_buffer;

				uint16_t vram = get_loopyregister(&nes->ppu->vram_addr);
				uint8_t inc = nes->ppu->ctrl.increment_mode? 32 : 1;
				set_loopyregister(&nes->ppu->vram_addr, vram + inc);

				break;
			default:
				perror("Invalid PPU Addr");
				exit(EXIT_FAILURE);
		}
	} else if (addr < 0x4014) {
		printf("[WARNING] Attempting to read from write-only APU address %04X; returning 0\n", addr);
	} else if (addr == 0x4015) {
		// TODO - implement APU register
	} else if (addr == 0x4016) {
		data = nes->controller1_state;
	} else if (addr == 0x4017) {
		data = nes->controller2_state;
	} else if (addr < 0x6000) {
		printf("[WARNING] Attempting to read from unimplemented expansion ROM address %04X; returning 0\n", addr);
	} else if (addr < 0x8000) {
		printf("[WARNING] Attempting to read from unimplemented SRAM address %04X; returning 0\n", addr);
	} else {
		data = cart_read_prg(nes->cart, addr - 0x8000);
	}
	return data;
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

uint8_t ppu_read(NES *nes, uint16_t addr) {
	uint8_t data = 0x00;
	addr &= 0x3FFF;

	if (addr < 0x2000) {

		data = cart_read_chr(nes->cart, addr);

	} else if (addr < 0x3F00) {

		addr &= 0x0FFF;
		switch (nes->cart->mirroring) {
			// credit to javidx9 (OneLoneCoder) for working this out
			case VERTICAL:
				if (addr < 0x0400)
					data = nes->ppu->nametable[0][addr & 0x03FF];
				else if (addr < 0x0800)
					data = nes->ppu->nametable[1][addr & 0x03FF];
				else if (addr < 0x0C00)
					data = nes->ppu->nametable[0][addr & 0x03FF];
				else
					data = nes->ppu->nametable[1][addr & 0x03FF];
				break;
			case HORIZONTAL:
				if (addr < 0x0400)
					data = nes->ppu->nametable[0][addr & 0x03FF];
				else if (addr < 0x0800)
					data = nes->ppu->nametable[0][addr & 0x03FF];
				else if (addr < 0x0C00)
					data = nes->ppu->nametable[1][addr & 0x03FF];
				else
					data = nes->ppu->nametable[1][addr & 0x03FF];
				break;
			default:
				fprintf(stderr, "[WARNING] Unsupported mirroring type in PPU READ function\n");
		}
	} else if (addr >= 0x3F00 && addr <= 0x3FFF) {
		addr &= 0x001F;
		if (addr == 0x0010) addr = 0x0000;
		if (addr == 0x0014) addr = 0x0004;
		if (addr == 0x0018) addr = 0x0008;
		if (addr == 0x001C) addr = 0x000C;
		data = nes->ppu->palette_table[addr] & (nes->ppu->mask.greyscale ? 0x30 : 0x3F);
	}
	return data;
}


void ppu_write(NES *nes, uint16_t addr, uint8_t value) {
	addr &= 0x3FFF;

	if (addr < 0x2000) {

		cart_write_chr(nes->cart, addr, value);

	} else if (addr < 0x3F00) {

		addr &= 0x0FFF;
		switch (nes->cart->mirroring) {
			// credit to javidx9 (OneLoneCoder) for working this out
			case VERTICAL:
				if (addr < 0x0400)
					nes->ppu->nametable[0][addr & 0x03FF] = value;
				else if (addr < 0x0800)
					nes->ppu->nametable[1][addr & 0x03FF] = value;
				else if (addr < 0x0C00)
					nes->ppu->nametable[0][addr & 0x03FF] = value;
				else
					nes->ppu->nametable[1][addr & 0x03FF] = value;
				break;
			case HORIZONTAL:
				if (addr < 0x0400)
					nes->ppu->nametable[0][addr & 0x03FF] = value;
				else if (addr < 0x0800)
					nes->ppu->nametable[0][addr & 0x03FF] = value;
				else if (addr < 0x0C00)
					nes->ppu->nametable[1][addr & 0x03FF] = value;
				else
					nes->ppu->nametable[1][addr & 0x03FF] = value;
				break;
			default:
				fprintf(stderr, "[WARNING] Unsupported mirroring type in PPU READ function\n");
		}
	} else if (addr >= 0x3F00 && addr <= 0x3FFF) {
		addr &= 0x001F;
		if (addr == 0x0010) addr = 0x0000;
		if (addr == 0x0014) addr = 0x0004;
		if (addr == 0x0018) addr = 0x0008;
		if (addr == 0x001C) addr = 0x000C;
		nes->ppu->palette_table[addr] = value;
	}
}

void load_cartridge(NES *nes, Cartridge *cart) {
	nes->cart = cart;
}

void reset(NES *nes) {
	reset_cpu(nes->cpu);
	reset_ppu(nes->ppu);
}

void clock(NES *nes) {
	(void) nes;
}

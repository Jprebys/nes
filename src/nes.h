#ifndef _NES_H
#define _NES_H

#include "ppu.h"
#include "cpu.h"
#include "cart.h"

typedef struct NES
{
	PPU       *ppu;
	CPU       *cpu;
	Cartridge *cart;
	uint8_t    controller1_state;
	uint8_t    controller2_state;

	size_t total_clocks;
} NES;

NES *init_nes();
void delete_nes(NES *);
void dump_nes_info(NES *, char *);

void cpu_write(NES *, uint16_t, uint8_t);
uint8_t cpu_read(NES *, uint16_t);
void ppu_write(NES *, uint16_t, uint8_t);
uint8_t ppu_read(NES *, uint16_t);

void oam_dma(NES *, uint8_t);

void load_cartridge(NES *, Cartridge *);
void reset(NES *);
void clock(NES *);

#endif
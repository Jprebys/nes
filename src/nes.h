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
} NES;

NES *init_nes();
void delete_nes(NES *);
void dump_nes_info(NES *, char *);

void mem_write(NES *, uint16_t, uint8_t);
uint8_t mem_read(NES *, uint16_t);


#endif
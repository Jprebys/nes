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

#endif
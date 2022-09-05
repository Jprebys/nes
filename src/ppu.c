#include "ppu.h"
#include <stdint.h>
#include <stdlib.h>

PPU *init_ppu()
{
	PPU *ppu = malloc(sizeof(PPU));
	return ppu;
}

void reset_ppu(PPU *ppu)
{
	memset(ppu, 0, sizeof(PPU));
}

uint8_t rgb_color_table[][3] = {
	{84,   84,  84}, {0,    30, 116}, {8, 16, 144}, {48, 0, 136}, {68, 0, 100}, {92, 0, 48}, {84, 4, 0}, {60, 24, 0}, {32, 42, 0}, {8, 58, 0}, {0, 64, 0}, {0, 60, 0}, {0, 50, 60}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
	{152, 150, 152}, {8,    76, 196}, {48, 50, 236}, {92, 30, 228}, {136, 20, 176}, {160, 20, 100}, {152, 34, 32}, {120, 60, 0}, {84, 90, 0}, {40, 114, 0}, {8, 124, 0}, {0, 118, 40}, {0, 102, 120}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
	{236, 238, 236}, {76,  154, 236}, {120, 124, 236}, {176, 98, 236}, {228, 84, 236}, {236, 88, 180}, {236, 106, 100}, {212, 136, 32}, {160, 170, 0}, {116, 196, 0}, {76, 208, 32}, {56, 204, 108}, {56, 180, 204}, {60, 60, 60}, {0, 0, 0}, {0, 0, 0},
	{236, 238, 236}, {168, 204, 236}, {188, 188, 236}, {212, 178, 236}, {236, 174, 236}, {236, 174, 212}, {236, 180, 176}, {228, 196, 144}, {204, 210, 120}, {180, 222, 120}, {168, 226, 144}, {152, 226, 180}, {160, 214, 228}, {160, 162, 160}, {0, 0, 0}, {0, 0, 0}
};

uint8_t get_ppumask(PPU *ppu)
{
	uint8_t result = 0x00;
	result |= ppu->mask.greyscale << 0;
	result |= ppu->mask.render_bg_left << 1;
	result |= ppu->mask.render_sprites_left << 2;
	result |= ppu->mask.render_bg << 3;
	result |= ppu->mask.render_sprites << 4;
	result |= ppu->mask.emph_red << 5;
	result |= ppu->mask.emph_green << 6;
	result |= ppu->mask.emph_blue << 7;
	return result;
}

void set_ppumask(PPU *ppu, uint8_t value)
{
	ppu->mask.greyscale = value & 1 << 0 ? 1 : 0;
	ppu->mask.render_bg_left = value & 1 << 1 ? 1 : 0;
	ppu->mask.render_sprites_left = value & 1 << 2 ? 1 : 0;
	ppu->mask.render_bg = value & 1 << 3 ? 1 : 0;
	ppu->mask.render_sprites = value & 1 << 4 ? 1 : 0;
	ppu->mask.emph_red = value & 1 << 5 ? 1 : 0;
	ppu->mask.emph_green = value & 1 << 6 ? 1 : 0;
	ppu->mask.emph_blue = value & 1 << 7 ? 1 : 0;
}

uint8_t get_ppuctrl(PPU *ppu)
{
	uint8_t result = 0x00;
	result |= ppu->ctrl.nametable_x << 0;
	result |= ppu->ctrl.nametable_y << 1;
	result |= ppu->ctrl.increment_mode << 2;
	result |= ppu->ctrl.pattern_sprite << 3;
	result |= ppu->ctrl.pattern_background << 4;
	result |= ppu->ctrl.sprite_size << 5;
	result |= ppu->ctrl.slave_mode << 6;
	result |= ppu->ctrl.enable_nmi << 7;
	return result;
}

void set_ppuctrl(PPU *ppu, uint8_t value)
{
	ppu->ctrl.nametable_x = value & (1 << 0) ? 1 : 0;
	ppu->ctrl.nametable_y = value & (1 << 1) ? 1 : 0;
	ppu->ctrl.increment_mode = value & (1 << 2) ? 1 : 0;
	ppu->ctrl.pattern_sprite = value & (1 << 3) ? 1 : 0;
	ppu->ctrl.pattern_background = value & (1 << 4) ? 1 : 0;
	ppu->ctrl.sprite_size = value & (1 << 5) ? 1 : 0;
	ppu->ctrl.slave_mode = value & (1 << 6) ? 1 : 0;
	ppu->ctrl.enable_nmi = value & (1 << 7) ? 1 : 0;
}

uint8_t get_ppustatus(PPU *ppu)
{
	uint8_t result = 0x00;
	result |= ppu->status.sprite_overflow << 5;
	result |= ppu->status.sprite_zero_hit << 6;
	result |= ppu->status.vertical_blank << 7;
	return result;
}

void set_ppustatus(PPU *ppu, uint8_t value)
{
	ppu->status.sprite_overflow = value & (1 << 5) ? 1 : 0;
	ppu->status.sprite_zero_hit = value & (1 << 6) ? 1 : 0;
	ppu->status.vertical_blank = value & (1 << 7) ? 1 : 0;
}

uint16_t get_loopyregister(LoopyRegister *reg) 
{
	uint16_t value = 0x0000;
	value |= (reg->coarse_x    << 0 ) & 0b0000000000011111;
	value |= (reg->coarse_y    << 5 ) & 0b0000001111100000;
	value |= (reg->nametable_x << 10) & 0b0000010000000000;
	value |= (reg->nametable_y << 11) & 0b0000100000000000;
	value |= (reg->fine_y      << 12) & 0b0111000000000000;
	return value;
}

void set_loopyregister(LoopyRegister *reg, uint16_t value) 
{
	reg->coarse_x    = (value >> 0 ) & 0b0000000000011111;
	reg->coarse_y    = (value >> 5 ) & 0b0000000000011111;
	reg->nametable_x = (value >> 10) & 0b0000000000000001;
	reg->nametable_y = (value >> 11) & 0b0000000000000001;
	reg->fine_y      = (value >> 12) & 0b0000000000000111;
}


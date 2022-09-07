#include "ppu.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

PPU *init_ppu()
{
	PPU *ppu = malloc(sizeof(PPU));
	return ppu;
}

void reset_ppu(PPU *ppu)
{
	memset(ppu, 0, sizeof(PPU));
}

uint8_t color_table[][3] = {
	{ 84,  84,  84}, {  0,  30, 116}, {  8,  16, 144}, { 48,   0, 136}, { 68,   0, 100}, { 92,   0,  48}, { 84,   4,   0}, { 60,  24,   0}, { 32,  42,   0}, {  8,  58,   0}, {  0,  64,   0}, {  0,  60,   0}, {  0,  50,  60}, {  0,   0,   0}, {  0,   0,   0}, {  0,   0,   0},
	{152, 150, 152}, {  8,  76, 196}, { 48,  50, 236}, { 92,  30, 228}, {136,  20, 176}, {160,  20, 100}, {152,  34,  32}, {120,  60,   0}, { 84,  90,   0}, { 40, 114,   0}, {  8, 124,   0}, {  0, 118,  40}, {  0, 102, 120}, {  0,   0,   0}, {  0,   0,   0}, {  0,   0,   0},
	{236, 238, 236}, { 76, 154, 236}, {120, 124, 236}, {176,  98, 236}, {228,  84, 236}, {236,  88, 180}, {236, 106, 100}, {212, 136,  32}, {160, 170,   0}, {116, 196,   0}, { 76, 208,  32}, { 56, 204, 108}, { 56, 180, 204}, { 60,  60,  60}, {  0,   0,   0}, {  0,   0,   0},
	{236, 238, 236}, {168, 204, 236}, {188, 188, 236}, {212, 178, 236}, {236, 174, 236}, {236, 174, 212}, {236, 180, 176}, {228, 196, 144}, {204, 210, 120}, {180, 222, 120}, {168, 226, 144}, {152, 226, 180}, {160, 214, 228}, {160, 162, 160}, {  0,   0,   0}, {  0,   0,   0}
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

void inc_scroll_x(PPU *ppu)
{
	if (ppu->mask.render_bg || ppu->mask.render_sprites) 
	{
		if (ppu->vram_addr.coarse_x == 31)
		{
			// Each nametable is 32x30 tiles, so if we are at an edge
			// we need to wrap x position around to 0 and flip table number
			ppu->vram_addr.coarse_x = 0;
			ppu->vram_addr.nametable_x = ppu->vram_addr.nametable_x ? 0 : 1;
		}
		else
		{
			ppu->vram_addr.coarse_x++;
		}
	}
}

void inc_scroll_y(PPU *ppu)
{
	if (ppu->mask.render_bg || ppu->mask.render_sprites) 
	{
		if (ppu->vram_addr.fine_y < 7)
		{
			ppu->vram_addr.fine_y++;
		}
		else
		{
			ppu->vram_addr.fine_y = 0;

			if (ppu->vram_addr.coarse_y == 29)
			{
				ppu->vram_addr.coarse_y = 0;
				ppu->vram_addr.nametable_y = ppu->vram_addr.nametable_y ? 0 : 1;
			}
			else if (ppu->vram_addr.coarse_y == 31)
			{
				ppu->vram_addr.coarse_y = 0;
			}
			else
			{
				ppu->vram_addr.coarse_y++;
			}
		}
	}
}

void trans_addr_x(PPU *ppu)
{
	if (ppu->mask.render_bg || ppu->mask.render_sprites) 
	{
		ppu->vram_addr.nametable_x = ppu->tram_addr.nametable_x;
		ppu->vram_addr.coarse_x    = ppu->tram_addr.coarse_x;
	}
}

void trans_addr_y(PPU *ppu)
{
	if (ppu->mask.render_bg || ppu->mask.render_sprites) 
	{
		ppu->vram_addr.fine_y      = ppu->tram_addr.fine_y;
		ppu->vram_addr.nametable_y = ppu->tram_addr.nametable_y;
		ppu->vram_addr.coarse_y    = ppu->tram_addr.coarse_y;
	}
}

void load_bg_shift(PPU *ppu)
{
	ppu->bg_shifter_pattern_lo = (ppu->bg_shifter_pattern_lo & 0xFF00) | ppu->bg_next_tile_lsb;
	ppu->bg_shifter_pattern_hi = (ppu->bg_shifter_pattern_hi & 0xFF00) | ppu->bg_next_tile_msb;

	ppu->bg_shifter_attrib_lo  = (ppu->bg_shifter_attrib_lo & 0xFF00) | ((ppu->bg_next_tile_attrib & 0b01) ? 0xFF : 0x00);
	ppu->bg_shifter_attrib_hi  = (ppu->bg_shifter_attrib_hi & 0xFF00) | ((ppu->bg_next_tile_attrib & 0b10) ? 0xFF : 0x00);
}

void update_shifters(PPU *ppu)
{
	if (ppu->mask.render_bg) 
	{
		// Shifting background tile pattern row
		ppu->bg_shifter_pattern_lo <<= 1;
		ppu->bg_shifter_pattern_hi <<= 1;

		// Shifting palette attributes by 1
		ppu->bg_shifter_attrib_lo <<= 1;
		ppu->bg_shifter_attrib_hi <<= 1;
	}
}

static bool on_screen(const PPU *ppu)
{
	int16_t cycle = ppu->cycle - 1;

	if ((cycle >= 0 && cycle < NES_RES_WIDTH) 
		&& (ppu->scanline >= 0 && ppu->scanline < NES_RES_HEIGHT))
		return true;
	return false;
}

uint8_t ppu_read(NES *, uint16_t);

void ppu_clock(PPU *ppu)
{
	if (ppu->scanline >= -1 && ppu->scanline < 240)
	{		
		if (ppu->scanline == 0 && ppu->cycle == 0)
		{
			ppu->cycle = 1;
		}

		if (ppu->scanline == -1 && ppu->cycle == 1)
		{
			ppu->status.vertical_blank = 0;
		}

		if ((ppu->cycle >= 2 && ppu->cycle < 258) || (ppu->cycle >= 321 && ppu->cycle < 338))
		{
			update_shifters(ppu);

			switch ((ppu->cycle - 1) % 8)
			{
				case 0:
					load_bg_shift(ppu);

					ppu->bg_next_tile_id = ppu_read(ppu->nes, 0x2000 | (get_loopyregister(&ppu->vram_addr) & 0x0FFF));

					break;
				case 2:
					ppu->bg_next_tile_attrib = ppu_read(ppu->nes, 0x23C0 | (ppu->vram_addr.nametable_y << 11) 
						                                 | (ppu->vram_addr.nametable_x << 10) 
						                                 | ((ppu->vram_addr.coarse_y >> 2) << 3) 
						                                 | (ppu->vram_addr.coarse_x >> 2));
					
					if (ppu->vram_addr.coarse_y & 0x02) ppu->bg_next_tile_attrib >>= 4;
					if (ppu->vram_addr.coarse_x & 0x02) ppu->bg_next_tile_attrib >>= 2;
					ppu->bg_next_tile_attrib &= 0x03;
					break;

				case 4: 
					ppu->bg_next_tile_lsb = ppu_read(ppu->nes, (ppu->ctrl.pattern_background << 12) 
						                       + ((uint16_t)ppu->bg_next_tile_id << 4) 
						                       + (ppu->vram_addr.fine_y) + 0);

					break;
				case 6:
					ppu->bg_next_tile_msb = ppu_read(ppu->nes, (ppu->ctrl.pattern_background << 12)
						                       + ((uint16_t)ppu->bg_next_tile_id << 4)
						                       + (ppu->vram_addr.fine_y) + 8);
					break;
				case 7:
					inc_scroll_x(ppu);
					break;
			}

		}

		if (ppu->cycle == 256)
		{
			inc_scroll_y(ppu);
		}

		if (ppu->cycle == 257)
		{
			load_bg_shift(ppu);
			trans_addr_x(ppu);
		}

		if (ppu->cycle == 338 || ppu->cycle == 340)
		{
			ppu->bg_next_tile_id = ppu_read(ppu->nes, 0x2000 | (get_loopyregister(&ppu->vram_addr) & 0x0FFF));
		}


		if (ppu->scanline == -1 && ppu->cycle >= 280 && ppu->cycle < 305)
		{
			trans_addr_y(ppu);
		}
	}

	if (ppu->scanline >= 241 && ppu->scanline < 261)
	{
		if (ppu->scanline == 241 && ppu->cycle == 1)
		{
			ppu->status.vertical_blank = 1;

			if (ppu->ctrl.enable_nmi) 
				ppu->nmi = true;
		}
	}
	uint8_t bg_pixel = 0x00; 
	uint8_t bg_palette = 0x00;

	if (ppu->mask.render_bg)
	{
		uint16_t bit_mux = 0x8000 >> ppu->fine_x;

		uint8_t p0_pixel = (ppu->bg_shifter_pattern_lo & bit_mux) > 0;
		uint8_t p1_pixel = (ppu->bg_shifter_pattern_hi & bit_mux) > 0;

		bg_pixel = (p1_pixel << 1) | p0_pixel;

		uint8_t bg_pal0 = (ppu->bg_shifter_attrib_lo & bit_mux) > 0;
		uint8_t bg_pal1 = (ppu->bg_shifter_attrib_hi & bit_mux) > 0;
		bg_palette = (bg_pal1 << 1) | bg_pal0;
	}
	uint32_t idx;
	bool c;
	if (c = on_screen(ppu)) 
	{
		idx = (ppu->scanline * NES_RES_WIDTH + ppu->cycle - 1) * 4;
		uint8_t *rgb = color_table[ppu_read(ppu->nes, 0x3F00 + (bg_palette << 2) + bg_pixel) & 0x3F];
		ppu->frame_pixels[idx]     = rgb[0];  // R
		ppu->frame_pixels[idx + 1] = rgb[1];  // G
		ppu->frame_pixels[idx + 2] = rgb[2];  // B
		ppu->frame_pixels[idx + 3] =   0xFF;  // A
		// if (rand() % 2) {
		// 	ppu->frame_pixels[idx]     = 0;  // R
		// 	ppu->frame_pixels[idx + 1] = 0;  // G
		// 	ppu->frame_pixels[idx + 2] = 0;  // B
		// 	ppu->frame_pixels[idx + 3] =   0xFF;  // A
		// } else {
		// 	ppu->frame_pixels[idx]     = 0xFF;  // R
		// 	ppu->frame_pixels[idx + 1] = 0xFF;  // G
		// 	ppu->frame_pixels[idx + 2] = 0xFF;  // B
		// 	ppu->frame_pixels[idx + 3] =   0xFF;  // A
		// }
	}

	ppu->cycle++;
	if (ppu->cycle >= 341)
	{
		ppu->cycle = 0;
		ppu->scanline++;
		if (ppu->scanline >= 261)
		{
			ppu->scanline = -1;
			ppu->frame_ready = true;
		}
	}

}




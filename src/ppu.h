#ifndef _PPU_H
#define _PPU_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct Mask 
{
	uint8_t greyscale: 1;            // 0: 0 normal color, 1 produce greyscale display
	uint8_t render_bg_left: 1;       // 1: 0 hide, 1 show background in leftmost 8 pixels of screen
	uint8_t render_sprites_left: 1;  // 2: 0 hide, 1 show sprites in leftmost 8 pixels of screen                        
	uint8_t render_bg: 1;            // 3: 1 show background
	uint8_t render_sprites: 1;       // 4: 1 show sprites
	uint8_t emph_red: 1;             // 5: emphasize red (green on PAL/Dendy)
	uint8_t emph_green: 1;           // 6: emphasize green (red on PAL/Dendy)
	uint8_t emph_blue: 1;            // 7: emphasize blue
} Mask;


typedef struct Controller
{
	uint8_t nametable_x: 1;          // 0: 1- Add 256 to the X scroll position
	uint8_t nametable_y: 1;          // 1: 1- Add 240 to the Y scroll position
	uint8_t increment_mode: 1;       // 2: VRAM address increment per CPU read/write of PPUDATA (0: add 1, going across; 1: add 32, going down)
	uint8_t pattern_sprite : 1;      // 3: Sprite pattern table address for 8x8 sprites (0: $0000; 1: $1000; ignored in 8x16 mode)
	uint8_t pattern_background : 1;  // 4: Background pattern table address (0: $0000; 1: $1000)
	uint8_t sprite_size : 1;         // 5: Sprite size (0: 8x8 pixels; 1: 8x16 pixels – see PPU OAM#Byte 1)
	uint8_t slave_mode : 1;          // 6: PPU master/slave select (0: read backdrop from EXT pins; 1: output color on EXT pins)
	uint8_t enable_nmi : 1;          // 7: Generate an NMI at the start of the vertical blanking interval (0: off; 1: on)
} Controller;


typedef struct Status
{
	/*
	    5: Sprite overflow. The intent was for this flag to be set
           whenever more than eight sprites appear on a scanline, but a
           hardware bug causes the actual behavior to be more complicated
           and generate false positives as well as false negatives; see
           PPU sprite evaluation. This flag is set during sprite
           evaluation and cleared at dot 1 (the second dot) of the
           pre-render line.
	*/
	uint8_t sprite_overflow : 1;     

	/* 
		6: Sprite 0 Hit.  Set when a nonzero pixel of sprite 0 overlaps
           a nonzero background pixel; cleared at dot 1 of the pre-render 
           line.  Used for raster timing.
	*/ 
	uint8_t sprite_zero_hit : 1;     

	/* 
		7: Vertical blank has started (0: not in vblank; 1: in vblank). 
		   Set at dot 1 of line 241 (the line *after* the post-render line); 
		   cleared after reading $2002 and at dot 1 of the pre-render line
	*/
	uint8_t vertical_blank : 1;      
} Status;


typedef struct LoopyRegister {
	uint16_t coarse_x : 5;
	uint16_t coarse_y : 5;
	uint16_t nametable_x : 1;
	uint16_t nametable_y : 1;
	uint16_t fine_y : 3;
} LoopyRegister;


typedef struct PPU
{
	uint8_t *chr_rom;
	size_t chr_rom_size;
	uint8_t pallette_table[32];
	uint8_t vram[2048];
	uint8_t oam_memory[256];

	Controller ctrl;  // PPUCTRL   $2000
	Mask       mask;  // PPUMASK   $2001
	Status   status;  // PPUSTATUS $2002

	uint8_t oam_addr; // OAMADDR   $2003
	uint8_t oam_data; // OAMDATA   $2004
	uint8_t scroll;   // PPUSCROLL $2005
	uint8_t addr;     // PPUADDR   $2006
	uint8_t data;     // PPUDATA   $2007
	uint8_t oam_dma;  // OAMDMA    $4014

	bool latch_set;
	uint8_t latch_value;

	uint8_t fine_x;

	bool address_latch;
	uint8_t ppu_data_buffer;

	int16_t scanline;
	int16_t cycle;

	LoopyRegister vram_addr;
	LoopyRegister tram_addr;
} PPU;

PPU *init_ppu();

uint8_t get_ppumask(PPU *);
void set_ppumask(PPU *, uint8_t);
uint8_t get_ppuctrl(PPU *);
void set_ppuctrl(PPU *, uint8_t);
uint8_t get_ppustatus(PPU *);
void set_ppustatus(PPU *, uint8_t);
uint16_t get_loopyregister(LoopyRegister *);
void set_loopyregister(LoopyRegister *, uint16_t);

#endif
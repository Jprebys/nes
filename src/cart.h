#ifndef _CART_H
#define _CART_H

#include <stdint.h>

typedef struct Cartridge
{
	uint8_t *prg_rom;
	size_t prg_rom_size;  // in bytes

	uint8_t *chr_rom;
	size_t chr_rom_size;  // in bytes

} Cartridge;

Cartridge *load_cart_from_file(char *);

#endif
#ifndef _CART_H
#define _CART_H

#include <stdint.h>
#include <stdbool.h>

#define TRAINER_SIZE    512

typedef struct Cartridge
{
	uint8_t *prg_rom;
	size_t prg_rom_size;  // in bytes

	uint8_t *chr_rom;
	size_t chr_rom_size;  // in bytes

	bool trainer_present;   // 1: 512-byte trainer at $7000-$71FF (stored before PRG data)
	bool contains_ram;      // 1: Cartridge contains battery-backed PRG RAM ($6000-7FFF) or other persistent memory
	bool mirroring;         // 0: horizontal (vertical arrangement) (CIRAM A10 = PPU A11) 1: vertical (horizontal arrangement) (CIRAM A10 = PPU A10)
	bool ignore_mirroring;  // 1: Ignore mirroring control or above mirroring bit; instead provide four-screen VRAM

	

	uint8_t trainer[TRAINER_SIZE];

	uint8_t mapper_id;


} Cartridge;

Cartridge *load_cart_from_file(char *);
void delete_cart(Cartridge *);

#endif
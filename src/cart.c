#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cart.h"

#define INES_HEADER_LEN 16
#define TRAINER_LEN     512
#define PRG_BLOCK_SIZE  16384
#define CHR_BLOCK_SIZE  8192
#define error_and_exit(X) do{perror(X); exit(EXIT_FAILURE);} while(0)

/*
The following was copied from: https://www.nesdev.org/wiki/INES

An iNES [.nes] file consists of the following sections, in order:

1. Header (16 bytes)
2. Trainer, if present (0 or 512 bytes)
3. PRG ROM data (16384 * x bytes)
4. CHR ROM data, if present (8192 * y bytes)
5. PlayChoice INST-ROM, if present (0 or 8192 bytes)
6. PlayChoice PROM, if present (16 bytes Data, 16 bytes CounterOut) (this is often missing, see PC10 ROM-Images for details)
7. Some ROM-Images additionally contain a 128-byte (or sometimes 127-byte) title at the end of the file.

The format of the header is as follows:

0-3: Constant $4E $45 $53 $1A ("NES" followed by MS-DOS end-of-file)
4: Size of PRG ROM in 16 KB units
5: Size of CHR ROM in 8 KB units (Value 0 means the board uses CHR RAM)
6: Flags 6 - Mapper, mirroring, battery, trainer
7: Flags 7 - Mapper, VS/Playchoice, NES 2.0
8: Flags 8 - PRG-RAM size (rarely used extension)
9: Flags 9 - TV system (rarely used extension)
10: Flags 10 - TV system, PRG-RAM presence (unofficial, rarely used extension)
11-15: Unused padding (should be filled with zero, but some rippers put their name across bytes 7-15)

*/

static const uint8_t iNES_SIG[4] = { 0x4E, 0x45, 0x53, 0x1A };


Cartridge *load_cart_from_file(char *fname)
{
	Cartridge *cart = malloc(sizeof(Cartridge));

	FILE *nes_file = fopen(fname, "r");
	if (nes_file == NULL)
		error_and_exit("Opening iNES file");

	uint8_t header[INES_HEADER_LEN];
	size_t bytes = fread(&header, 1, INES_HEADER_LEN, nes_file);
	if (bytes != INES_HEADER_LEN)
		error_and_exit("Reading iNES header");

	// check signature
	if (memcmp(&header, &iNES_SIG, 4))
		error_and_exit("Invalid iNES signature");

	cart->prg_rom_size = header[4] * PRG_BLOCK_SIZE;
	if (header[5] == 0)
		header[5] = 1;
	cart->chr_rom_size = header[5] * CHR_BLOCK_SIZE;
	cart->prg_rom = malloc(cart->prg_rom_size);
	cart->chr_rom = malloc(cart->chr_rom_size);

	cart->contains_ram     = header[6] & (1 << 1) ? true : false;
	cart->trainer_present  = header[6] & (1 << 2) ? true : false;

	bool v_mirroring = header[6] & (1 << 0) ? true : false;
	bool four_screen = header[6] & (1 << 3) ? true : false;

	if (four_screen) {
		cart->mirroring = FOUR_SCREEN;
	} else if (v_mirroring) {
		cart->mirroring = VERTICAL;
	} else {
		cart->mirroring = HORIZONTAL;
	}

	cart->mapper_id = 0;
	cart->mapper_id |= ((header[6] >> 4) & 0x0F);
	cart->mapper_id |= (header[7] & 0xF0);

	if (cart->mapper_id & 0x10) {
		fprintf(stderr, "Error: we do not currently support iNES 2.0 files");
		exit(EXIT_FAILURE);
	}

	if (cart->trainer_present) {
		int result = fseek(nes_file, TRAINER_LEN, SEEK_CUR);
		if (result == -1) {
			perror("Fseek nes file");
			exit(EXIT_FAILURE);
		}
	}

	size_t prg_rom_read = fread(cart->prg_rom, 1, cart->prg_rom_size, nes_file);
	if (prg_rom_read != cart->prg_rom_size) {
		perror("Loading PRG ROM");
		exit(EXIT_FAILURE);
	}

	size_t chr_rom_read = fread(cart->chr_rom, 1, cart->chr_rom_size, nes_file);
	if (chr_rom_read != cart->chr_rom_size) {
		perror("Loading CHR ROM");
		exit(EXIT_FAILURE);
	}

	fprintf(stdout, "Successfully loaded iNES file with mapper %02X, %u prg banks, and %u chr banks\n",
		    cart->mapper_id, header[4], header[5]);

	return cart;
}

void delete_cart(Cartridge *cart)
{
	free(cart->prg_rom);
	free(cart->chr_rom);
	free(cart);
}


uint8_t cart_read_prg(Cartridge *cart, uint16_t addr) 
{
	if (cart->prg_rom_size == 0x4000 && addr >= 0x4000)
		addr %= 0x4000;
	return cart->prg_rom[addr];
}

uint8_t cart_read_chr(Cartridge *cart, uint16_t addr)
{
	if (addr > cart->chr_rom_size) {
		fprintf(stderr, "[WARNING] Attempting to read addr %04X outside of CHR ROM boundary (size %04lx); returning 0\n",
			    addr, cart->chr_rom_size);
		return 0x00;
	}
	return cart->chr_rom[addr];
}

void cart_write_chr(Cartridge *cart, uint16_t addr, uint8_t value)
{
	if (!cart->contains_ram) {
		fprintf(stderr, "[WARNING] Attempting to write to CHR ROM");

	} else {
		cart->chr_rom[addr] = value;
	}
}


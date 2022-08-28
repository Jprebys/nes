#include <stdlib.h>

#include "nes.h"
#include "cpu.h"
#include "cart.h"

NES *init_nes()
{
	NES *nes = malloc(sizeof(NES));
	nes->cpu = init_cpu();
	nes->ppu = init_ppu();
	nes->cart = NULL;
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
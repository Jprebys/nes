#include "raylib.h"

#include "nes.h"


#define NES_RES_WIDTH  256
#define NES_RES_HEIGHT 240
#define NES_SCALE      3

int main(void)
{
	const int width  = 1150;
	const int height = NES_RES_HEIGHT * 3 + 30;

	NES *nes = init_nes();

	InitWindow(width, height, "Example window - font loading");
	EnableEventWaiting();

	char nes_info_buffer[2048];
	nes_info_buffer[0] = '\0';

	Font font = LoadFontEx("resources/fonts/kongtext.ttf", 13, 0, 250);

	SetTargetFPS(60);

	Vector2 text_info_pos = {(float)NES_RES_WIDTH  * NES_SCALE + 10,
	                         0};

	while(!WindowShouldClose())
	{
		BeginDrawing();
		DrawFPS(10, 10);
		ClearBackground(BLACK);

		dump_nes_info(nes, nes_info_buffer);
		// mouse_pos = GetMousePosition();
		DrawTextEx(font, nes_info_buffer, text_info_pos, (float)font.baseSize, 2, RAYWHITE);

		EndDrawing();

	}

	UnloadFont(font);
	CloseWindow();
	delete_nes(nes);

	return 0;
}
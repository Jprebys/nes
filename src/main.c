#include <assert.h>
#include <stdbool.h>
#include <raylib.h>
#include "nes.h"

#define NES_SCALE      3
#define BUTTON_COUNT   8

int main(int argc, char **argv)
{
	const int width  = 1150;
	const int height = NES_RES_HEIGHT * 3 + 30;
	const int buttons[BUTTON_COUNT] = { KEY_W, KEY_A, KEY_S, KEY_D, KEY_J, KEY_K, KEY_N, KEY_M };
	const char *button_names[] = { "Up", "Left", "Down", "Right",
		                          "A", "B", "Select", "Start" };
	bool keys_pressed[BUTTON_COUNT];

	NES *nes = init_nes();
	if (argc == 2)
		nes->cart = load_cart_from_file(argv[1]);
	else
		nes->cart = load_cart_from_file("resources/Donkey Kong (World) (Rev A).nes");
	reset_cpu(nes->cpu);

	InitWindow(width, height, "jNES Emulator");
	DisableEventWaiting();

	char nes_info_buffer[2048];
	nes_info_buffer[0] = '\0';

	Font font = LoadFontEx("resources/fonts/kongtext.ttf", 13, 0, 250);

	SetTargetFPS(60);

	Vector2 text_info_pos = {(float)NES_RES_WIDTH * NES_SCALE + 40, 0};
	Vector2 origin        = {15.0f, 15.0f};
	RenderTexture2D target = LoadRenderTexture(NES_RES_WIDTH, NES_RES_HEIGHT);
	target.texture.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;


	while(!WindowShouldClose())
	{
		
		clock(nes);

		// BeginTextureMode(target);
		// EndTextureMode();

		UpdateTexture(target.texture, nes->ppu->frame_pixels);

		BeginDrawing();
		ClearBackground(BLACK);
		DrawTextureEx(target.texture, origin, 0.0f, 3.0f, WHITE);
		DrawFPS(10, 10);

		Vector2 button_pressed_pos = { 20.0f, 50.0f };

		for (size_t i = 0; i < BUTTON_COUNT; i++)
		{
			keys_pressed[i] = IsKeyDown(buttons[i]);
			button_pressed_pos.y += 50.0f;
			if (keys_pressed[i])		
				DrawTextEx(font, button_names[i], button_pressed_pos, 30.0f, 1, RAYWHITE);
		}

		dump_nes_info(nes, nes_info_buffer);
		DrawTextEx(font, nes_info_buffer, text_info_pos, (float)font.baseSize, 1, RAYWHITE);

		EndDrawing();

	}

	UnloadTexture(target.texture);
	UnloadFont(font);
	CloseWindow();
	delete_nes(nes);

	return 0;
}
#include <stdbool.h>
#include <raylib.h>
#include "nes.h"

#define NES_SCALE      3
#define BUTTON_COUNT   8

int main(void)
{
	const int width  = 1150;
	const int height = NES_RES_HEIGHT * 3 + 30;
	const int buttons[BUTTON_COUNT] = { KEY_W, KEY_A, KEY_S, KEY_D, KEY_J, KEY_K, KEY_N, KEY_M };
	const char *button_names[] = { "Up", "Left", "Down", "Right",
		                          "A", "B", "Select", "Start" };
	bool keys_pressed[BUTTON_COUNT];

	NES *nes = init_nes();
	nes->cart = load_cart_from_file("resources/Balloon Fight (USA).nes");

	InitWindow(width, height, "jNES Emulator");
	DisableEventWaiting();

	char nes_info_buffer[2048];
	nes_info_buffer[0] = '\0';

	Font font = LoadFontEx("resources/fonts/kongtext.ttf", 13, 0, 250);

	SetTargetFPS(120);

	Vector2 text_info_pos = {(float)NES_RES_WIDTH * NES_SCALE + 40, 0};
	Vector2 origin        = {15.0f, 15.0f};
	RenderTexture2D target = LoadRenderTexture(NES_RES_WIDTH, NES_RES_HEIGHT);
	target.texture.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
	uint8_t rseed = 85;
	uint8_t gseed = 170;
	uint8_t bseed = 255;
	uint8_t pixels[PIXELS_LEN - 10];

	while(!WindowShouldClose())
	{
		clock(nes);

		BeginTextureMode(target);
		// ClearBackground(WHITE);
		for (int i = 0; i < PIXELS_LEN; ++i)
		{
			switch (i % 4) {
				case 0:
					pixels[i] = (i % 0xFF) + rseed;
					break;
				case 1:
					pixels[i] = (i % 0xFF) + gseed;
					break;
				case 2:
					pixels[i] = (i % 0xFF) + bseed;
					break;
				case 3:
					pixels[i] = 0xFF;
					break;
			}
		}
		rseed++; gseed++; bseed++;
		EndTextureMode();

		UpdateTexture(target.texture, pixels);

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

	UnloadFont(font);
	CloseWindow();
	delete_nes(nes);

	return 0;
}
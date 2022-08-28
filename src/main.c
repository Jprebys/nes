#include <stdbool.h>
#include <raylib.h>
#include "nes.h"


#define NES_RES_WIDTH  256
#define NES_RES_HEIGHT 240
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

	InitWindow(width, height, "Example window - font loading");
	DisableEventWaiting();

	char nes_info_buffer[2048];
	nes_info_buffer[0] = '\0';

	Font font = LoadFontEx("resources/fonts/kongtext.ttf", 13, 0, 250);

	SetTargetFPS(60);

	Vector2 text_info_pos = {(float)NES_RES_WIDTH  * NES_SCALE + 10, 0};



	while(!WindowShouldClose())
	{
		BeginDrawing();
		DrawFPS(10, 10);
		ClearBackground(BLACK);

		Vector2 button_pressed_pos = { 20.0f, 50.0f };

		for (size_t i = 0; i < BUTTON_COUNT; i++)
		{
			keys_pressed[i] = IsKeyDown(buttons[i]);
			button_pressed_pos.y += 50.0f;
			if (keys_pressed[i])		
				DrawTextEx(font, button_names[i], button_pressed_pos, 30.0f, 1, RAYWHITE);
		}

		dump_nes_info(nes, nes_info_buffer);
		// mouse_pos = GetMousePosition();
		DrawTextEx(font, nes_info_buffer, text_info_pos, (float)font.baseSize, 1, RAYWHITE);

		EndDrawing();
	}

	UnloadFont(font);
	CloseWindow();
	delete_nes(nes);

	return 0;
}
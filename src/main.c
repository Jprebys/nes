
#include "raylib.h"

int main(void)
{
	const int width  = 800;
	const int height = 600;

	InitWindow(width, height, "Example window - font loading");

	char msg[] = "Hello, World! Here is some text";

	Font font = LoadFontEx("resources/fonts/JetBrainsMono-Regular.ttf", 32, 0, 250);

	SetTargetFPS(30);

	while(!WindowShouldClose())
	{
		BeginDrawing();

		ClearBackground(RAYWHITE);

		DrawTextEx(font, msg, (Vector2){ 20.0f, 100.0f }, (float)font.baseSize, 2, MAROON);

		EndDrawing();

	}

	UnloadFont(font);
	CloseWindow();

	return 0;
}
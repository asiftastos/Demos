/*
*	SDL Window Demo
*/

#include <stdio.h>
#define DM_WINDOW_IMPLEMENTATION
#include <dmWindow.h>

DmWindow dw;

void handleKeyboard(SDL_KeyboardEvent *ev)
{
	if (ev->keysym.sym == SDLK_ESCAPE)
		dw.running = false;
}

int main(int argc, const char* argv[])
{
	DmWindowParams dparams = { 1024, 768, NOAPI, "SDL Window" };

	if (InitWindow(&dparams, &dw) > 0)
		return 1;

	dw.running = true;

	SDL_Event e = { 0 };
	while (dw.running)
	{
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
			case SDL_QUIT:
				dw.running = false;
				break;
			case SDL_KEYDOWN:
				handleKeyboard(&e.key);
				break;
			default:
				break;
			}
		}
	}

	CloseWindow(&dw);

	return 0;
}
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

int main(int argc, const char** argv)
{
	DmWindowParams dparams = { "SDL Window", 1024, 768 };

	if (initWindow(&dparams, &dw) > 0)
		return 1;

	/*	SYSTEM INFO */
	int cpus = SDL_GetCPUCount();
	int ram = SDL_GetSystemRAM();
	SDL_Log("CPUs: %d\tRAM: %d MB", cpus, ram);
	
	/*	FILESYSTEM INFO  */
	SDL_Log("Working Dir: %s\n", SDL_GetBasePath());

	SDL_Event e = { 0 };
	dw.running = true;
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
				handleKeyboard(&e);
				break;
			default:
				break;
			}
		}
	}

	quitWindow(&dw);

	return 0;
}
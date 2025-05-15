/*
*	SDL Window
*/

#include <stdio.h>
#define DM_WINDOW_IMPLEMENTATION
#include "demos/common/dmWindow.h"

DmWindow dw;
SDL_Renderer* sdlrenderer = NULL;

void handleKeyboard(SDL_KeyboardEvent* ev)
{
	if (ev->key == SDLK_ESCAPE)
		dw.running = false;
	if (ev->key == SDLK_F1)
		GrabMouse(&dw);
	if (ev->key == SDLK_F2)
		ReleaseMouse(&dw);
}

int main(int argc, const char* argv[])
{
	DmWindowParams dparams = {
		.width = 1024,
		.height = 768,
		.api = NOAPI,
		.title = "SDL Renderer"
	};

	if (InitWindow(&dparams, &dw) > 0)
		return 1;

	dw.keyboardHandler = handleKeyboard;

	int rdDrvCount = SDL_GetNumRenderDrivers();
	int driverIndex = -1;
	SDL_Log("Found %d render drivers\n", rdDrvCount);
	for (int i = 0; i < rdDrvCount; i++)
	{
		const char* renderDriverName = SDL_GetRenderDriver(i);
		SDL_Log("[%d]: %s\n", i, renderDriverName);
		if (SDL_strcmp(renderDriverName, "direct3d11") == 0)
		{
			driverIndex = i;
		}
	}

	sdlrenderer = SDL_CreateRenderer(dw.window, NULL);
	if (!sdlrenderer)
	{
		SDL_Log("SDL Renderer error: %s\n", SDL_GetError());
		CloseWindow(&dw);
		return 1;
	}

	//SDL_RendererInfo renderinfo;
	//SDL_GetRendererInfo(sdlrenderer, &renderinfo);
	//SDL_Log("Current Renderer: %s\n", renderinfo.name);

	dw.running = true;
	SDL_Event e = { 0 };
	while (dw.running)
	{
		ProcessEvents(&dw, &e);

		SDL_SetRenderDrawColor(sdlrenderer, 50, 50, 50, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(sdlrenderer);

		SDL_SetRenderDrawColor(sdlrenderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
		SDL_RenderLine(sdlrenderer, 10, 10, 200, 10);

		SDL_SetRenderDrawColor(sdlrenderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
		SDL_FRect r1 = { 10, 30, 200, 100 };
		SDL_RenderRect(sdlrenderer, &r1);

		SDL_SetRenderDrawColor(sdlrenderer, 0, 0, 255, SDL_ALPHA_OPAQUE);
		SDL_FRect r2 = { 10, 200, 200, 100 };
		SDL_RenderFillRect(sdlrenderer, &r2);

		SDL_RenderPresent(sdlrenderer);
	}

	if (sdlrenderer)
		SDL_DestroyRenderer(sdlrenderer);

	//sdl
	QuitWindow(&dw);

	return 0;
}

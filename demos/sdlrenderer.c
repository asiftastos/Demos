/*
*	SDL Renderer Demo
*/

#include <stdio.h>
#define DM_WINDOW_IMPLEMENTATION
#include <dmWindow.h>

DmWindow dw;
SDL_Renderer* sdlrenderer = NULL;

void handleKeyboard(SDL_KeyboardEvent* ev)
{
	if (ev->keysym.sym == SDLK_ESCAPE)
		dw.running = false;
}

int main(int argc, const char** argv)
{
	DmWindowParams dparams = { 1280, 900, NOAPI, "SDL Renderer" };

	if (InitWindow(&dparams, &dw) > 0)
		return 1;

	dw.keyboardHandler = handleKeyboard;

	int rdDrvCount = SDL_GetNumRenderDrivers();
	int driverIndex = -1;
	for (int i = 0; i < rdDrvCount; i++)
	{
		SDL_RendererInfo rdInfo;
		SDL_GetRenderDriverInfo(i, &rdInfo);
		SDL_Log("[%d]: %s\n", i, rdInfo.name);
		if (SDL_strcmp(rdInfo.name, "direct3d11") == 0)
		{
			driverIndex = i;
		}
	}

	sdlrenderer = SDL_CreateRenderer(dw.window, driverIndex, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!sdlrenderer)
	{
		SDL_Log("SDL Renderer error: %s\n", SDL_GetError());
		CloseWindow(&dw);
		return 1;
	}
	
	SDL_RendererInfo renderinfo;
	SDL_GetRendererInfo(sdlrenderer, &renderinfo);
	SDL_Log("Current Renderer: %s\n", renderinfo.name);

	SDL_Event e = { 0 };
	dw.running = true;
	while (dw.running)
	{
		ProcessEvents(&dw, &e);

		SDL_SetRenderDrawColor(sdlrenderer, 50, 50, 50, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(sdlrenderer);

		SDL_SetRenderDrawColor(sdlrenderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
		SDL_RenderDrawLine(sdlrenderer, 10, 10, 200, 10);

		SDL_SetRenderDrawColor(sdlrenderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
		SDL_Rect r1 = {10, 30, 200, 100};
		SDL_RenderDrawRect(sdlrenderer, &r1);

		SDL_SetRenderDrawColor(sdlrenderer, 0, 0, 255, SDL_ALPHA_OPAQUE);
		SDL_Rect r2 = { 10, 200, 200, 100 };
		SDL_RenderFillRect(sdlrenderer, &r2);

		SDL_RenderPresent(sdlrenderer);
	}

	if (sdlrenderer)
		SDL_DestroyRenderer(sdlrenderer);

	CloseWindow(&dw);

	return 0;
}
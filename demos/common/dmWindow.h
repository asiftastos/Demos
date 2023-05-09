/*
*	SDL Window stuff
*/
#pragma once

#include <stdbool.h>
#include <SDL.h>

typedef struct DmWindowParams {
	char* title;
	int width;
	int height;
}DmWindowParams;

typedef struct DmWindow {
	SDL_Window* window;
	bool running;
}DmWindow;

int initWindow(DmWindowParams* params, DmWindow* dmW);
void quitWindow(DmWindow* dmW);

#ifdef DM_WINDOW_IMPLEMENTATION

int initWindow(DmWindowParams* params, DmWindow* dmW)
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		const char* errormsg = SDL_GetError();
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "[ERROR]: %s\n", errormsg);
		return 1;
	}
	SDL_Log("SDL2 initialized!!\n");

	dmW->window = SDL_CreateWindow(params->title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, params->width, params->height, SDL_WINDOW_RESIZABLE);
	if (!dmW->window)
	{
		const char* errormsg = SDL_GetError();
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "[ERROR]: %s\n", errormsg);
		SDL_Quit();
		return 1;
	}

	return 0;
}

void quitWindow(DmWindow* dmW)
{
	SDL_DestroyWindow(dmW->window);
	SDL_Quit();
	SDL_Log("SDL2 quit!!\n");
}

#endif // DM_WINDOW_IMPLEMENTATION

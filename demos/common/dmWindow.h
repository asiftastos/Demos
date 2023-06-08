/*
*	SDL Window stuff
*/
#pragma once

#include <stdbool.h>
#include <SDL.h>

typedef enum DmGraphicsApi {
	NOAPI,
	OPENGL,
	VULKAN
}DmGraphicsApi;

typedef struct DmWindowParams {
	int width;
	int height;
	DmGraphicsApi api;
	char* title;
}DmWindowParams;

typedef struct DmWindow {
	bool running;
	SDL_Window* window;
}DmWindow;

int InitWindow(DmWindowParams* params, DmWindow* dmW);
void CloseWindow(DmWindow* dmW);

#ifdef DM_WINDOW_IMPLEMENTATION

int InitWindow(DmWindowParams* params, DmWindow* dmW)
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		const char* errormsg = SDL_GetError();
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "[ERROR]: %s\n", errormsg);
		return 1;
	}
	SDL_Log("SDL2 initialized!!\n");

	Uint32 windowFlags = SDL_WINDOW_RESIZABLE;
	if (params->api == OPENGL) {
		windowFlags |= SDL_WINDOW_OPENGL;
	}
	else if(params->api == VULKAN)
	{
		windowFlags |= SDL_WINDOW_VULKAN;
	}

	dmW->window = SDL_CreateWindow(params->title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, params->width, params->height, windowFlags);
	if (!dmW->window)
	{
		const char* errormsg = SDL_GetError();
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "[ERROR]: %s\n", errormsg);
		SDL_Quit();
		return 1;
	}
	SDL_Log("SDL2 window created!\n");

	/*	Video Drivers  */
	int vidDrvCount = SDL_GetNumVideoDrivers();
	SDL_Log("Video drivers: %d\n", vidDrvCount);
	for (int i = 0; i < vidDrvCount; i++)
	{
		SDL_Log("[%d]: %s\n", i, SDL_GetVideoDriver(i));
	}

	/*  Video Displays  */
	int vidDisplaysCount = SDL_GetNumVideoDisplays();
	SDL_Log("Video displays: %d\n", vidDisplaysCount);
	for (int i = 0; i < vidDisplaysCount; i++)
	{
		SDL_Log("[%d]: %s\n", i, SDL_GetDisplayName(i));
	}

	/*	SYSTEM INFO */
	int cpus = SDL_GetCPUCount();
	int ram = SDL_GetSystemRAM();
	SDL_Log("CPUs: %d\tRAM: %d MB", cpus, ram);

	/*	FILESYSTEM INFO  */
	SDL_Log("Working Dir: %s\n", SDL_GetBasePath());

	return 0;
}

void CloseWindow(DmWindow* dmW)
{
	SDL_DestroyWindow(dmW->window);
	SDL_Quit();
	SDL_Log("SDL2 closed!!\n");
}

#endif // DM_WINDOW_IMPLEMENTATION

/*
*	SDL Window stuff
*/
#pragma once

#include <stdbool.h>
#include "SDL2/SDL.h"

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

typedef void (*QUITHANDLER)(void);
typedef void(*WINDOWHANDLER)(SDL_WindowEvent* event);
typedef void(*KEYBOARDHANDLER)(SDL_KeyboardEvent* event);
typedef void(*MOUSEBUTTONHANDLER)(SDL_MouseButtonEvent* event);

typedef struct DmWindow {
	bool running;
	SDL_RWops* fp;
	SDL_Window* window;
	bool isMouseGrabbed;
	QUITHANDLER quitHandler;
	WINDOWHANDLER windowHandler;
	KEYBOARDHANDLER keyboardHandler;
	MOUSEBUTTONHANDLER mouseButtonHandler;
}DmWindow;

int InitWindow(DmWindowParams* params, DmWindow* dmW);
void QuitWindow(DmWindow* dmW);

void ProcessEvents(DmWindow* dmW, SDL_Event* event);

void GrabMouse(DmWindow* dmW);
void ReleaseMouse(DmWindow* dmW);

/*
* ------------------------------------
*	IMPLEMENTATION
* ------------------------------------
*/
#ifdef DM_WINDOW_IMPLEMENTATION

static void LogOutputCallback(void* userdata, int category, SDL_LogPriority priority, const char* message) {
	SDL_RWops* fp = (SDL_RWops*)userdata;
	size_t len = strlen(message);
	char buff[512];
	strcpy(buff, message);
	buff[len] = '\n';
	SDL_RWwrite(fp, buff, sizeof(char), len + 1);
}

int InitWindow(DmWindowParams* params, DmWindow* dmW)
{
#ifdef _DEBUG
	SDL_LogSetAllPriority(SDL_LOG_PRIORITY_INFO);
#endif // _DEBUG

	dmW->fp = SDL_RWFromFile("BasicWindow.log", "w");
	if (!dmW->fp)
		return 1;

	SDL_LogSetOutputFunction(LogOutputCallback, (void*)dmW->fp);

	SDL_LogError(SDL_LOG_CATEGORY_ERROR, "[ERROR]: test");

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		const char* errormsg = SDL_GetError();
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "[ERROR]: %s", errormsg);
		SDL_RWclose(dmW->fp);
		return 1;
	}

	SDL_Log("SDL2 initialized!!");

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
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "[ERROR]: %s", errormsg);
		SDL_Quit();
		SDL_RWclose(dmW->fp);
		return 1;
	}
	SDL_Log("SDL2 window created!");

	/*	Video Drivers  */
	int vidDrvCount = SDL_GetNumVideoDrivers();
	SDL_Log("Video drivers: %d", vidDrvCount);
	for (int i = 0; i < vidDrvCount; i++)
	{
		SDL_Log("[%d]: %s", i, SDL_GetVideoDriver(i));
	}

	/*  Video Displays  */
	int vidDisplaysCount = SDL_GetNumVideoDisplays();
	SDL_Log("Video displays: %d", vidDisplaysCount);
	for (int i = 0; i < vidDisplaysCount; i++)
	{
		SDL_Log("[%d]: %s", i, SDL_GetDisplayName(i));
	}

	/*	SYSTEM INFO */
	int cpus = SDL_GetCPUCount();
	int ram = SDL_GetSystemRAM();
	SDL_Log("CPUs: %d\tRAM: %d MB", cpus, ram);

	/*	FILESYSTEM INFO  */
	SDL_Log("Working Dir: %s", SDL_GetBasePath());

	/*  EVENT HANDLERS */
	dmW->quitHandler = NULL;
	dmW->keyboardHandler = NULL;
	dmW->windowHandler = NULL;
	dmW->mouseButtonHandler = NULL;

	dmW->isMouseGrabbed = false;

	return 0;
}

void QuitWindow(DmWindow* dmW)
{
	SDL_DestroyWindow(dmW->window);
	SDL_Quit();
	SDL_Log("SDL2 closed!!");
	SDL_RWclose(dmW->fp);
}

void ProcessEvents(DmWindow* dmW, SDL_Event* event)
{
	while (SDL_PollEvent(event))
	{
		switch (event->type)
		{
		case SDL_QUIT:
			dmW->running = false;
			if (dmW->quitHandler)  dmW->quitHandler();
			break;
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			if (dmW->keyboardHandler)  dmW->keyboardHandler(&event->key);
			break;
		case SDL_WINDOWEVENT:
			if (dmW->windowHandler)  dmW->windowHandler(&event->window);
			break;
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			if (dmW->mouseButtonHandler)  dmW->mouseButtonHandler(&event->button);
			break;
		default:
			break;
		}
	}
}

void GrabMouse(DmWindow* dmW)
{
	int w, h;
	SDL_GetWindowSize(dmW->window, &w, &h);
	SDL_Rect r = (SDL_Rect){ (int)(w / 2), (int)(h / 2), 1, 1 };
	SDL_SetWindowMouseRect(dmW->window, &r);
	SDL_SetWindowMouseGrab(dmW->window, SDL_TRUE);
	SDL_ShowCursor(SDL_DISABLE);
	dmW->isMouseGrabbed = true;
}

void ReleaseMouse(DmWindow* dmW)
{
	SDL_SetWindowMouseRect(dmW->window, NULL);
	SDL_SetWindowMouseGrab(dmW->window, SDL_FALSE);
	SDL_ShowCursor(SDL_ENABLE);
	dmW->isMouseGrabbed = false;
}

#endif // DM_WINDOW_IMPLEMENTATION

/*
*	SDL Window stuff
*/
#pragma once

#include <stdbool.h>
#include "SDL3/SDL.h"

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
	SDL_IOStream* fp;
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
	SDL_IOStream* fp = (SDL_IOStream*)userdata;
	size_t len = strlen(message);
	char buff[512];
	strcpy(buff, message);
	buff[len] = '\n';
	SDL_WriteIO(fp, buff, sizeof(char) * (len + 1));
}

static int InitLog(DmWindow* dmW, const char* title) {
	const char* ext = ".log";
	char filename[64];
	SDL_memset(filename, 0, sizeof(filename));
	
	size_t titlelen = SDL_strlen(title) + 1;
	SDL_strlcpy(filename, title, titlelen);
	
	size_t extensionlen = SDL_strlen(ext) + 1;
	SDL_strlcat(filename, ext, titlelen + extensionlen);

	dmW->fp = SDL_IOFromFile(filename, "w");
	if (!dmW->fp) {
		const char* error = SDL_GetError();
		printf("[IO ERROR]: %s", error);
		return 1;
	}
	return 0;
}

int InitWindow(DmWindowParams* params, DmWindow* dmW)
{
#ifdef _DEBUG
	SDL_SetLogPriorities(SDL_LOG_PRIORITY_INFO);
#endif // _DEBUG

	if(InitLog(dmW, params->title) > 0)
		return 1;

	SDL_SetLogOutputFunction(LogOutputCallback, (void*)dmW->fp);

	SDL_LogError(SDL_LOG_CATEGORY_ERROR, "[ERROR]: test");

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		const char* errormsg = SDL_GetError();
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "[ERROR]: %s", errormsg);
		SDL_CloseIO(dmW->fp);
		return 1;
	}

	SDL_Log("SDL3 initialized!!");

	Uint32 windowFlags = SDL_WINDOW_RESIZABLE;
	if (params->api == OPENGL) {
		windowFlags |= SDL_WINDOW_OPENGL;
	}
	else if(params->api == VULKAN)
	{
		windowFlags |= SDL_WINDOW_VULKAN;
	}

	dmW->window = SDL_CreateWindow(params->title, params->width, params->height, windowFlags);
	if (!dmW->window)
	{
		const char* errormsg = SDL_GetError();
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "[ERROR]: %s", errormsg);
		SDL_Quit();
		SDL_CloseIO(dmW->fp);
		return 1;
	}
	SDL_Log("SDL3 window created!");

	/*	Video Drivers  */
	int vidDrvCount = SDL_GetNumVideoDrivers();
	SDL_Log("Video drivers: %d", vidDrvCount);
	for (int i = 0; i < vidDrvCount; i++)
	{
		SDL_Log("[%d]: %s", i, SDL_GetVideoDriver(i));
	}

	/*  Video Displays  */
	int vidDisplaysCount;
	SDL_DisplayID* displays = SDL_GetDisplays(&vidDisplaysCount);
	SDL_Log("Video displays: %d", vidDisplaysCount);
	for (int i = 0; i < vidDisplaysCount; i++)
	{
		SDL_Log("[%d]: %s", i, SDL_GetDisplayName(displays[i]));
	}
	if(displays)
		SDL_free(displays);

	/*	SYSTEM INFO */
	int cpus = SDL_GetNumLogicalCPUCores();
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
	SDL_Log("SDL3 closed!!");
	SDL_CloseIO(dmW->fp);
}

void ProcessEvents(DmWindow* dmW, SDL_Event* event)
{
	while (SDL_PollEvent(event))
	{
		switch (event->type)
		{
		case SDL_EVENT_QUIT:
			dmW->running = false;
			if (dmW->quitHandler)  dmW->quitHandler();
			break;
		case SDL_EVENT_KEY_DOWN:
		case SDL_EVENT_KEY_UP:
			if (dmW->keyboardHandler)  dmW->keyboardHandler(&event->key);
			break;
		case SDL_EVENT_WINDOW_SHOWN:
		case SDL_EVENT_WINDOW_HIDDEN:
		case SDL_EVENT_WINDOW_EXPOSED:
		case SDL_EVENT_WINDOW_MOVED:
		case SDL_EVENT_WINDOW_RESIZED:
		case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
		case SDL_EVENT_WINDOW_METAL_VIEW_RESIZED:
		case SDL_EVENT_WINDOW_MINIMIZED:
		case SDL_EVENT_WINDOW_MAXIMIZED:
		case SDL_EVENT_WINDOW_RESTORED:
		case SDL_EVENT_WINDOW_MOUSE_ENTER:
		case SDL_EVENT_WINDOW_MOUSE_LEAVE:
		case SDL_EVENT_WINDOW_FOCUS_GAINED:
		case SDL_EVENT_WINDOW_FOCUS_LOST:
		case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
		case SDL_EVENT_WINDOW_HIT_TEST:
		case SDL_EVENT_WINDOW_ICCPROF_CHANGED:
		case SDL_EVENT_WINDOW_DISPLAY_CHANGED:
		case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED:
		case SDL_EVENT_WINDOW_SAFE_AREA_CHANGED:
		case SDL_EVENT_WINDOW_OCCLUDED:
		case SDL_EVENT_WINDOW_ENTER_FULLSCREEN:
		case SDL_EVENT_WINDOW_LEAVE_FULLSCREEN:
		case SDL_EVENT_WINDOW_DESTROYED:
			if (dmW->windowHandler)  dmW->windowHandler(&event->window);
			break;
		case SDL_EVENT_MOUSE_BUTTON_DOWN:
		case SDL_EVENT_MOUSE_BUTTON_UP:
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
	SDL_SetWindowMouseGrab(dmW->window, true);
	SDL_HideCursor();
	dmW->isMouseGrabbed = true;
}

void ReleaseMouse(DmWindow* dmW)
{
	SDL_SetWindowMouseRect(dmW->window, NULL);
	SDL_SetWindowMouseGrab(dmW->window, false);
	SDL_ShowCursor();
	dmW->isMouseGrabbed = false;
}

#endif // DM_WINDOW_IMPLEMENTATION

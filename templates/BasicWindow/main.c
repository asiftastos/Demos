/*
*	SDL Window
*/

// Standard library
#include <stdio.h>

//Demo Commons
#define DM_VEC_IMPLEMENTATION
#include "demos/common/dmVec.h"
#define DM_MAP_IMPLEMENTATION
#include "demos/common/dmMap.h"
#define DM_LOGGER_IMPLEMENTATION
#include "demos/common/dmLogger.h"
#define DM_INI_IMPLEMENTATION
#include "demos/common/dmIni.h"

#define DM_WINDOW_IMPLEMENTATION
#include "demos/common/dmWindow.h"

DmWindow dw;

void handleKeyboard(SDL_KeyboardEvent* ev)
{
	if (ev->keysym.sym == SDLK_ESCAPE)
		dw.running = false;
	if (ev->keysym.sym == SDLK_F1)
		GrabMouse(&dw);
	if (ev->keysym.sym == SDLK_F2)
		ReleaseMouse(&dw);
}

int main(int argc, const char* argv[])
{
	DmWindowParams dparams = {
		.width = 1024,
		.height = 768,
		.api = NOAPI,
		.title = "SDL Window"
	};

	if (InitWindow(&dparams, &dw) > 0)
		return 1;

	dw.keyboardHandler = handleKeyboard;

	dw.running = true;

	SDL_Event e = { 0 };
	while (dw.running)
	{
		ProcessEvents(&dw, &e);
	}

	//sdl
	QuitWindow(&dw);

	return 0;
}

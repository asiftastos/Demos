/*
*	SDL OpenGL Triangle Demo
*/

#include <stdio.h>
#define DM_WINDOW_IMPLEMENTATION
#include <dmWindow.h>
#define DM_RENDERER_IMPLEMENTATION
#include <dmRenderer.h>

DmWindow dw;

void handleKeyboard(SDL_KeyboardEvent* ev)
{
	if (ev->keysym.sym == SDLK_ESCAPE)
		dw.running = false;
}

int main(int argc, const char** argv)
{
	DmWindowParams dparams = { "SDL Window", 1024, 768, OPENGL };

	if (initWindow(&dparams, &dw) > 0)
		return 1;

	initRenderer(&dw);

	SDL_GL_SetSwapInterval(1);

	glClearColor(0.0f, 0.0f, 0.8f, 1.0f);

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

		glClear(GL_COLOR_BUFFER_BIT);

		SDL_GL_SwapWindow(dw.window);
	}

	destroyRenderer();
	quitWindow(&dw);

	return 0;
}

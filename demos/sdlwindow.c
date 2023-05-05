/*
*	SDL Window Demo
*/

#include <stdio.h>
#include <stdbool.h>
#include <SDL.h>

typedef struct Demo {
	SDL_Window* window;
	bool running;
}Demo;

int main(int argc, const char** argv)
{
	Demo d;

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		const char* errormsg = SDL_GetError();
		SDL_LogError(SDL_LOG_CATEGORY_ERROR ,"[ERROR]: %s\n", errormsg);
		return 1;
	}
	SDL_Log("SDL2 initialized!!\n");

	d.window = SDL_CreateWindow("SDL Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024, 768, SDL_WINDOW_RESIZABLE);
	if (!d.window)
	{
		const char* errormsg = SDL_GetError();
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "[ERROR]: %s\n", errormsg);
		SDL_Quit();
		return 1;
	}

	d.running = true;
	while (d.running)
	{
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
			case SDL_QUIT:
				d.running = false;
				break;
			default:
				break;
			}
		}
	}

	SDL_DestroyWindow(d.window);
	SDL_Quit();
	SDL_Log("SDL2 quit!!\n");
	return 0;
}
#pragma once

#include <stdbool.h>
#include <dmWindow.h>

bool initRenderer(DmWindow* win);
void destroyRenderer();

#ifdef DM_RENDERER_IMPLEMENTATION

#include <glad.h>

static SDL_GLContext glContext = NULL;

bool initRenderer(DmWindow* win)
{
	glContext = SDL_GL_CreateContext(win->window);
	int r = SDL_GL_MakeCurrent(win->window, glContext);

	r = gladLoadGLLoader(SDL_GL_GetProcAddress);

	return true;
}

void destroyRenderer()
{
	SDL_GL_DeleteContext(glContext);
}

#endif // DM_RENDERER_IMPLEMENTATION

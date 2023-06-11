#pragma once

#include <stdbool.h>
#include <dmWindow.h>
#include <HandmadeMath.h>

typedef struct DMRenderer
{
	SDL_GLContext glContext;
	HMM_Mat4 ortho;
	HMM_Mat4 projection;
}DMRenderer;

DMRenderer* InitRenderer(DmWindow* win);
void DestroyRenderer(DMRenderer* dmRenderer);

void BeginDraw();
void EndDraw(DmWindow* win);

void BeginDraw2D();
void EndDraw2D();

void BeginDraw3D();

//depth test
void DepthTest(bool enable);

//blend
void Blend(bool enable);

/* 
* ------------------------------
*	IMPLEMENTATION
* ------------------------------
*/
#ifdef DM_RENDERER_IMPLEMENTATION

#include <stdlib.h>

#define GLAD_MALLOC malloc
#define GLAD_FREE free

#define GLAD_GL_IMPLEMENTATION
#include <glad.h>

DMRenderer* InitRenderer(DmWindow* win)
{
	DMRenderer* dmRenderer = (DMRenderer*)calloc(1, sizeof(DMRenderer));
	if (!dmRenderer)
	{
		SDL_Log("Failed to create Renderer\n");
		return NULL;
	}

	dmRenderer->glContext = SDL_GL_CreateContext(win->window);
	int r = SDL_GL_MakeCurrent(win->window, dmRenderer->glContext);

	r = gladLoadGL(SDL_GL_GetProcAddress);

	SDL_GL_SetSwapInterval(1);

	const char* version = glGetString(GL_VERSION);
	SDL_Log("OpenGL: %s\n", version);
	const char* glsl = glGetString(GL_SHADING_LANGUAGE_VERSION);
	SDL_Log("GLSL: %s\n", glsl);
	const char* renderer = glGetString(GL_RENDERER);
	SDL_Log("Vendor: %s\n", renderer);
	const char* vendor = glGetString(GL_VENDOR);
	SDL_Log("Vendor: %s\n", vendor);

	SDL_Log("OpenGL renderer initialized!\n");

	return dmRenderer;
}

void DestroyRenderer(DMRenderer* dmRenderer)
{
	SDL_GL_DeleteContext(dmRenderer->glContext);

	if (dmRenderer)
		free(dmRenderer);

	SDL_Log("OpenGL renderer destroyed!\n");
}

void BeginDraw()
{
	glClear(GL_COLOR_BUFFER_BIT);
}

void EndDraw(DmWindow* win)
{
	SDL_GL_SwapWindow(win->window);
}

void BeginDraw2D()
{
	DepthTest(false);
	Blend(true);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void EndDraw2D()
{
	Blend(false);
}

void BeginDraw3D()
{
	DepthTest(true);
}

void DepthTest(bool enable)
{
	if (enable)
	{
		glEnable(GL_DEPTH_TEST);
	}
	else
	{
		glDisable(GL_DEPTH_TEST);
	}
}

void Blend(bool enable)
{
	if (enable)
	{
		glEnable(GL_BLEND);
	}
	else
	{
		glDisable(GL_BLEND);
	}
}

#endif // DM_RENDERER_IMPLEMENTATION

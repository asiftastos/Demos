#pragma once

#include <stdbool.h>
#include "demos/common/dmWindow.h"
#include "demos/common/HandmadeMath.h"

typedef struct DMAttribute
{
	int index;
	int elementCount;
	int offset;
}DMAttribute;

typedef struct DMVao
{
	unsigned int id;
	int primitiveCount;
	int stride;
	int primitiveType;
}DMVao;

typedef struct DMVertexBuffer
{
	unsigned int id;
	unsigned int bindType;
	unsigned int size;
}DMVertexBuffer;

typedef struct DMRenderer
{
	SDL_GLContext glContext;
	HMM_Mat4 ortho;
	HMM_Mat4 projection;
}DMRenderer;

/*
* -----------------------------------------
*	Renderer and GL State
* -------------------------------------------
*/
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
* --------------------------------------------
*	Vertex Array Object
* --------------------------------------------
*/

void CreateVao(DMVao* vao, int stride);
void DeleteVao(DMVao* vao);
void EnableVao(DMVao* vao); //If vao is NULL unbind else bind vao
void EnableVaoAttribute(DMVao* vao, DMAttribute attr);
void DrawVao(DMVao* vao, int first);
void DrawElementsVao(DMVao* vao, unsigned int elementType);

/*
* --------------------------------------------
*	Vertex Buffer
* --------------------------------------------
*/

void CreateBuffer(DMVertexBuffer* buffer, unsigned int size, unsigned int bind);
void DeleteBuffer(DMVertexBuffer* buffer);
void EnableBuffer(DMVertexBuffer* buffer, unsigned int bindType); //if buffer is NULL unbind any buffer, bindtype must be set to the bind buffer type (eg GL_ARRAY_BUFFER)
void UploadBufferData(DMVertexBuffer* buffer, void* data, unsigned int drawType);

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
#include "demos/opengl/glad.h"

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
	SDL_GL_DestroyContext(dmRenderer->glContext);

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

/*
*   Vertex Array Object
*/

void CreateVao(DMVao* vao, int stride)
{
	if (vao == NULL)
		vao = (DMVao*)calloc(1, sizeof(DMVao));

	glCreateVertexArrays(1, &(vao->id));
	EnableVao(vao);

	vao->stride = stride;
}

void DeleteVao(DMVao* vao)
{
	if (vao == NULL)
		return;

	EnableVao(NULL);
	glDeleteVertexArrays(1, &vao->id);
}

void EnableVao(DMVao* vao)
{
	if (vao == NULL)
	{
		glBindVertexArray(0);
		return;
	}
	glBindVertexArray(vao->id);
}

void EnableVaoAttribute(DMVao* vao, DMAttribute attr)
{
	glEnableVertexAttribArray(attr.index);
	glVertexAttribPointer(attr.index, attr.elementCount, GL_FLOAT, GL_FALSE, vao->stride, (void*)attr.offset);
}

void DrawVao(DMVao* vao, int first)
{
	EnableVao(vao);
	glDrawArrays(vao->primitiveType, first, vao->primitiveCount);
	EnableVao(NULL);
}

void DrawElementsVao(DMVao* vao, unsigned int elementType)
{
	EnableVao(vao);
	glDrawElements(vao->primitiveType, vao->primitiveCount, elementType, NULL);
	EnableVao(NULL);
}

/*
*	Vertex Buffer
*/

void CreateBuffer(DMVertexBuffer* buffer, unsigned int size, unsigned int bind)
{
	if (buffer == NULL)
		buffer = (DMVertexBuffer*)calloc(1, sizeof(DMVertexBuffer));

	glCreateBuffers(1, &(buffer->id));
	
	buffer->bindType = bind;
	buffer->size = size;
	
	EnableBuffer(buffer, 0);
}

void DeleteBuffer(DMVertexBuffer* buffer)
{
	if (buffer == NULL)
		return;

	//EnableBuffer(NULL, GL_ARRAY_BUFFER);
	glDeleteBuffers(1, &(buffer->id));
}

void EnableBuffer(DMVertexBuffer* buffer, unsigned int bindType)
{
	if (buffer == NULL)
	{
		glBindBuffer(bindType, 0);
		return;
	}
	glBindBuffer(buffer->bindType, buffer->id);
}

void UploadBufferData(DMVertexBuffer* buffer, void* data, unsigned int drawType)
{
	glBufferData(buffer->bindType, buffer->size, data, drawType);
}

#endif // DM_RENDERER_IMPLEMENTATION

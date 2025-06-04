/*
*	SDL Window
*/

#include <stdio.h>
#define DM_WINDOW_IMPLEMENTATION
#include "demos/common/dmWindow.h"
#define DM_RENDERER_IMPLEMENTATION
#include "demos/opengl/dmRenderer.h"
#define DM_ASSETS_IMPLEMENTATION
#include "demos/common/dmAssets.h"

int windowWidth = 1920;
int windowHeight = 1080;

DmWindow dw;
DMRenderer* renderer;
DMVao vao;
DMVertexBuffer vbo;
DMVertexBuffer ibo;

DMShader* simple2dShader;

float alpha = 1.0f;
HMM_Vec3 position = { 100.0f, 200.0f, 0.0f };

void createTriangle()
{
	float verts[] = {
		0.0f, 150.0f, 1.0f, 1.0f, 0.0f, 0.0f,
		300.0f, 150.0f, 1.0f, 0.0f, 1.0f, 0.0f,
		150.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f
	};

	Uint32 indices[] = {
		0,1,2
	};

	CreateVao(&vao, 6 * sizeof(float));

	CreateBuffer(&vbo, 18 * sizeof(float), GL_ARRAY_BUFFER);
	UploadBufferData(&vbo, verts, GL_STATIC_DRAW);

	CreateBuffer(&ibo, 3 * sizeof(Uint32), GL_ELEMENT_ARRAY_BUFFER);
	UploadBufferData(&ibo, indices, GL_STATIC_DRAW);

	EnableVaoAttribute(&vao, (DMAttribute) { 0, 3, 0 });
	EnableVaoAttribute(&vao, (DMAttribute) { 1, 3, 3 * sizeof(float) });

	vao.primitiveCount = 3;
}

void handleKeyboard(SDL_KeyboardEvent* ev)
{
	if (ev->key == SDLK_ESCAPE)
		dw.running = false;
	if (ev->key == SDLK_F1)
		GrabMouse(&dw);
	if (ev->key == SDLK_F2)
		ReleaseMouse(&dw);
}

void handleWindow(SDL_WindowEvent* ev)
{
	switch (ev->type)
	{
	case SDL_EVENT_WINDOW_RESIZED:
		glViewport(0, 0, ev->data1, ev->data2);
		windowWidth = ev->data1;
		windowHeight = ev->data2;
		break;
	default:
		break;
	}
}

int main(int argc, const char* argv[])
{
	DmWindowParams dparams = {
		.width = windowWidth,
		.height = windowHeight,
		.api = OPENGL,
		.title = "SDL GL Triangle"
	};

	if (InitWindow(&dparams, &dw) > 0)
		return 1;

	dw.keyboardHandler = handleKeyboard;
	dw.windowHandler = handleWindow;

	renderer = InitRenderer(&dw);
	if (!renderer)
	{
		CloseWindow(&dw);
		return 1;
	}

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	DMShaderInfo shaderInfo = {
		.name = "simple2d",
		.vertexShaderFile = "assets/shaders/simple2d.vert",
		.fragmentShaderFile = "assets/shaders/simple2d.frag"
	};

	simple2dShader = newShader(&shaderInfo);
	
	createTriangle();

	renderer->ortho = HMM_Orthographic_LH_NO(0.0f, (float)windowWidth, (float)windowHeight, 0.0f, 0.1f, 1.0f);
	HMM_Mat4 model = HMM_M4D(1.0f); //identity
	model = HMM_Translate(position);

	HMM_Vec3 color = HMM_V3(1.0f, 0.0f, 0.0f);

	dw.running = true;
	SDL_Event e = { 0 };
	while (dw.running)
	{
		ProcessEvents(&dw, &e);

		int x, y;
		int x1, y1;
		SDL_GetRelativeMouseState(&x1, &y1);
		Uint32 buttons = SDL_GetMouseState(&x, &y);
		HMM_Mat4 modelInverse = HMM_InvTranslate(model);
		HMM_Vec4 mp = HMM_MulM4V4(modelInverse, HMM_V4((float)x, (float)y, 0.0f, 1.0f)); //translate mouse coords to model coords
		if (mp.X > 0 && mp.X < 300 && mp.Y > 0 && mp.Y < 150)
		{
			alpha = 0.6f;
			if (buttons & SDL_BUTTON_LMASK)
			{
				//printf("[%d,%d]: %d,%d\n", x, y, x1, y1);
				HMM_Vec3 pmdiff = HMM_SubV3(position, HMM_V3(x, y, 0.0f));
				pmdiff = HMM_AddV3(pmdiff, HMM_V3(x1, y1, 0.0f));
				position = HMM_AddV3(HMM_V3(x, y, 0.0f), pmdiff);
				model = HMM_Translate(position);
			}
		}
		else
		{
			alpha = 1.0f;
		}

		BeginDraw();

		BeginDraw2D();

		Use(simple2dShader);

		GLint mloc = glGetUniformLocation(simple2dShader->id, "model");
		GLint ploc = glGetUniformLocation(simple2dShader->id, "proj");
		GLint aloc = glGetUniformLocation(simple2dShader->id, "alpha");

		glUniformMatrix4fv(mloc, 1, GL_FALSE, &model);
		glUniformMatrix4fv(ploc, 1, GL_FALSE, &renderer->ortho);
		glUniform1f(aloc, alpha);
		//glUniform3fv(cloc, 1, &color);

		EnableBuffer(&ibo, 0);
		vao.primitiveType = GL_TRIANGLES;
		DrawElementsVao(&vao, GL_UNSIGNED_INT);

		EndDraw2D();

		EndDraw(&dw);
	}

	deleteShader(simple2dShader);

	DeleteBuffer(&ibo);
	DeleteBuffer(&vbo);
	DeleteVao(&vao);
	DestroyRenderer(renderer);

	//sdl
	QuitWindow(&dw);

	return 0;
}

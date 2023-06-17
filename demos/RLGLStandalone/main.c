/*
*	RLGL Standalone
*/

// NOTE: rlgl can be configured just re-defining the following values:
//#define RL_DEFAULT_BATCH_BUFFER_ELEMENTS   8192    // Default internal render batch elements limits
//#define RL_DEFAULT_BATCH_BUFFERS              1    // Default number of batch buffers (multi-buffering)
//#define RL_DEFAULT_BATCH_DRAWCALLS          256    // Default number of batch draw calls (by state changes: mode, texture)
//#define RL_DEFAULT_BATCH_MAX_TEXTURE_UNITS    4    // Maximum number of textures units that can be activated on batch drawing (SetShaderValueTexture())
//#define RL_MAX_MATRIX_STACK_SIZE             32    // Maximum size of internal Matrix stack
//#define RL_MAX_SHADER_LOCATIONS              32    // Maximum number of shader locations supported
//#define RL_CULL_DISTANCE_NEAR              0.01    // Default projection matrix near cull distance
//#define RL_CULL_DISTANCE_FAR             1000.0    // Default projection matrix far cull distance

#include <stdio.h>

#include "demos/common/raymath.h"
/*
#define LOG_WARNING 4
#define TRACELOG
#define TRACELOG(level, ...)	fprintf(stdout, __VA_ARGS__)  //needs function with va_start/va_end to append line ending (\n)
#define TRACELOGD(...)	fprintf(stdout, __VA_ARGS__)
#define RLGL_SHOW_GL_DETAILS_INFO  //need to define TRACELOG(...)
*/
#define RLGL_IMPLEMENTATION
#include "demos/common/rlgl.h"

#define DM_WINDOW_IMPLEMENTATION
#include "demos/common/dmWindow.h"

DmWindow dw;
SDL_GLContext glContext;

void handleKeyboard(SDL_KeyboardEvent* ev)
{
	if (ev->keysym.sym == SDLK_ESCAPE)
		dw.running = false;
	if (ev->keysym.sym == SDLK_F1)
		GrabMouse(&dw);
	if (ev->keysym.sym == SDLK_F2)
		ReleaseMouse(&dw);
}

static void DrawRectangleV(Vector2 pos, Vector2 size)
{
	rlBegin(RL_TRIANGLES);

	rlColor4ub(255, 0, 0, 255);

	rlVertex2f(pos.x, pos.y);
	rlVertex2f(pos.x, pos.y + size.y);
	rlVertex2f(pos.x + size.x, pos.y + size.y);

	rlVertex2f(pos.x, pos.y);
	rlVertex2f(pos.x + size.x, pos.y + size.y);
	rlVertex2f(pos.x + size.x, pos.y);

	rlEnd();
}

int main(int argc, const char* argv[])
{
	DmWindowParams dparams = {
		.width = 1024,
		.height = 768,
		.api = OPENGL,
		.title = "RLGL Standalone"
	};

	if (InitWindow(&dparams, &dw) > 0)
		return 1;

	dw.keyboardHandler = handleKeyboard;

	glContext = SDL_GL_CreateContext(dw.window);
	int r = SDL_GL_MakeCurrent(dw.window, glContext);
	SDL_GL_SetSwapInterval(1);

	//rlgl
	rlLoadExtensions(SDL_GL_GetProcAddress);
	rlglInit(dparams.width, dparams.height);
	rlViewport(0, 0, dparams.width, dparams.height);
	rlMatrixMode(RL_PROJECTION);
	rlLoadIdentity();
	rlOrtho(0, dparams.width, dparams.height, 0, 0.0f, 1.0f);
	rlMatrixMode(RL_MODELVIEW);
	rlLoadIdentity();
	rlClearColor(100, 100, 100, 255);

	dw.running = true;

	SDL_Event e = { 0 };
	while (dw.running)
	{
		ProcessEvents(&dw, &e);

		rlClearScreenBuffers();

		// can't i use another math library? maybe with convert functions
		//Matrix model = MatrixTranslate(0.0, 100.0, 0.0);
		//rlSetMatrixModelview(model);
		//or
		rlPushMatrix();
		rlTranslatef(10.0f, 100.0f, 0.0f);
		DrawRectangleV((Vector2) { 10.0f, 10.0f }, (Vector2) { 200.0f, 40.0f });
		rlPopMatrix();

		rlDrawRenderBatchActive();

		SDL_GL_SwapWindow(dw.window);
	}

	//rlgl
	rlglClose();

	//sdl
	SDL_GL_DeleteContext(glContext);
	QuitWindow(&dw);

	return 0;
}

/*
*	SDL OpenGL Triangle Demo
*/

#include <stdio.h>
#define DM_WINDOW_IMPLEMENTATION
#include <dmWindow.h>
#define DM_RENDERER_IMPLEMENTATION
#include <dmRenderer.h>
#define DM_ASSETS_IMPLEMENTATION
#include <dmAssets.h>

int windowWidth = 1024;
int windowHeight = 768;

DmWindow dw;
DMRenderer* renderer;
GLuint vao;
GLuint vbo;
GLuint ibo;
GLuint shader;

float alpha = 1.0f;
HMM_Vec3 position = { 100.0f, 200.0f, 0.0f };

void createTriangle()
{
	float verts[] = {
		0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f,
		300.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
		150.0f, 150.0f, 1.0f, 0.0f, 0.0f, 1.0f
	};

	Uint32 indices[] = {
		0,1,2
	};

	glCreateVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glCreateBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(float), verts, GL_STATIC_DRAW);

	glCreateBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * sizeof(Uint32), indices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 3 * sizeof(float));
}

void handleKeyboard(SDL_KeyboardEvent* ev)
{
	if (ev->keysym.sym == SDLK_ESCAPE)
		dw.running = false;
}

void handleWindow(SDL_WindowEvent* ev)
{
	switch (ev->type)
	{
	case SDL_WINDOWEVENT_RESIZED:
		glViewport(0, 0, ev->data1, ev->data2);
		windowWidth = ev->data1;
		windowHeight = ev->data2;
		break;
	default:
		break;
	}
}

int main(int argc, const char** argv)
{
	DmWindowParams dparams = { windowWidth, windowHeight, OPENGL, "SDL Window" };

	if (InitWindow(&dparams, &dw) > 0)
		return 1;

	renderer = initRenderer(&dw);
	if (!renderer)
	{
		CloseWindow(&dw);
		return 1;
	}

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	GLuint vsh = loadShader("assets/shaders/simple2d.vert", GL_VERTEX_SHADER);
	GLuint fsh = loadShader("assets/shaders/simple2d.frag", GL_FRAGMENT_SHADER);
	shader = createProgram(vsh, fsh, "simple2d");

	createTriangle();

	renderer->ortho = HMM_Orthographic_LH_NO(0.0f, (float)windowWidth, 0.0f, (float)windowHeight, 0.1f, 1000.0f);
	HMM_Mat4 model = HMM_M4D(1.0f); //identity
	model = HMM_Translate(position);

	HMM_Vec3 color = HMM_V3(1.0f, 0.0f, 0.0f);

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
				handleKeyboard(&e.key);
				break;
			case SDL_WINDOWEVENT:
				handleWindow(&e.window);
				break;
			default:
				break;
			}
		}

		int x, y;
		int x1, y1;
		SDL_GetRelativeMouseState(&x1, &y1);
		Uint32 buttons = SDL_GetMouseState(&x, &y);
		y = windowHeight - y; //invert from top-left to bottom-left
		HMM_Mat4 modelInverse = HMM_InvTranslate(model);
		HMM_Vec4 mp = HMM_MulM4V4(modelInverse, HMM_V4((float)x, (float)y, 0.0f, 1.0f)); //translate mouse coords to model coords
		if (mp.X > 0 && mp.X < 300 && mp.Y > 0 && mp.Y < 150)
		{
			alpha = 0.6f;
			if (buttons & SDL_BUTTON_LMASK)
			{
				//why x axis is reverse?
				printf("%d,%d\n", x1, y1);
				HMM_Vec3 pmdiff = HMM_SubV3(HMM_V3(x, y, 0.0f), position);
				pmdiff = HMM_AddV3(pmdiff, HMM_V3(x1, y1, 0.0f));
				position = HMM_SubV3(HMM_V3(x, y, 0.0f), pmdiff);
				model = HMM_Translate(position);
			}
		}
		else
		{
			alpha = 1.0f;
		}

		beginDraw();

		beginDraw2D();

		glUseProgram(shader);
		GLint mloc = glGetUniformLocation(shader, "model");
		GLint ploc = glGetUniformLocation(shader, "proj");
		GLint aloc = glGetUniformLocation(shader, "alpha");
		//GLint cloc = glGetUniformLocation(shader, "color");

		glUniformMatrix4fv(mloc, 1, GL_FALSE, &model);
		glUniformMatrix4fv(ploc, 1, GL_FALSE, &renderer->ortho);
		glUniform1f(aloc, alpha);
		//glUniform3fv(cloc, 1, &color);

		glBindVertexArray(vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, NULL);

		endDraw2D();

		endDraw(&dw);
	}

	glDeleteBuffers(1, &ibo);
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
	glDeleteProgram(shader);
	destroyRenderer(renderer);
	CloseWindow(&dw);

	return 0;
}

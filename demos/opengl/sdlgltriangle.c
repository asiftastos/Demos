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

DmWindow dw;
DMRenderer* renderer;
GLuint vao;
GLuint vbo;
GLuint ibo;
GLuint shader;

GLuint loadShader(const char* filename, int shaderType)
{
	GLuint id = 0;

	int length = 0;
	char* src = loadFileText(filename, &length);

	if (src != NULL)
	{
		id = glCreateShader(shaderType);
		glShaderSource(id, 1, &src, NULL);
		glCompileShader(id);
		freeText(src);

		GLint status = GL_TRUE;
		glGetShaderiv(id, GL_COMPILE_STATUS, &status);
		
		if (status == GL_FALSE)
		{
			int infolength = 0;
			glGetShaderiv(id, GL_INFO_LOG_LENGTH, &infolength);

			char* infolog = (char*)calloc(infolength, sizeof(char));
			glGetShaderInfoLog(id, infolength, NULL, infolog);

			SDL_Log("Shader compile error [%s]: %s\n", filename, infolog);
			free(infolog);

			id = 0;
		}
	}

	return id;
}

GLuint createProgram(GLuint vsh, GLuint fsh, const char* name)
{
	GLuint id = 0;

	if (vsh > 0 && fsh > 0)
	{
		id = glCreateProgram();

		glAttachShader(id, vsh);
		glAttachShader(id, fsh);

		glLinkProgram(id);

		GLint status = GL_TRUE;
		glGetProgramiv(id, GL_LINK_STATUS, &status);

		if (status == GL_FALSE)
		{
			int infolength = 0;
			glGetProgramiv(id, GL_INFO_LOG_LENGTH, &infolength);

			char* infolog = (char*)calloc(infolength, sizeof(char));
			glGetProgramInfoLog(id, infolength, NULL, infolog);

			SDL_Log("Program link error [%s]: %s\n", name, infolog);
			free(infolog);

			id = 0;
		}

		glDetachShader(id, vsh);
		glDetachShader(id, fsh);

		glDeleteShader(vsh);
		glDeleteShader(fsh);
	}

	return id;
}

void createTriangle()
{
	float verts[] = {
		0.0f, 0.0f, 1.0f,
		100.0f, 0.0f, 1.0f,
		50.0f, 50.0f, 1.0f
	};

	Uint32 indices[] = {
		0,1,2
	};

	glCreateVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glCreateBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), verts, GL_STATIC_DRAW);

	glCreateBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * sizeof(Uint32), indices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
}

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

	renderer = initRenderer(&dw);
	if (!renderer)
	{
		quitWindow(&dw);
		return 1;
	}

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	GLuint vsh = loadShader("assets/shaders/simple2d.vert", GL_VERTEX_SHADER);
	GLuint fsh = loadShader("assets/shaders/simple2d.frag", GL_FRAGMENT_SHADER);
	shader = createProgram(vsh, fsh, "simple2d");

	createTriangle();

	renderer->ortho = HMM_Orthographic_LH_NO(0.0f, 1024.0f, 0.0f, 768.0f, 0.1f, 1000.0f);
	HMM_Mat4 model = HMM_M4D(1.0f); //identity

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
				handleKeyboard(&e);
				break;
			default:
				break;
			}
		}

		beginDraw();

		glUseProgram(shader);
		GLint mloc = glGetUniformLocation(shader, "model");
		GLint ploc = glGetUniformLocation(shader, "proj");
		GLint aloc = glGetUniformLocation(shader, "alpha");
		GLint cloc = glGetUniformLocation(shader, "color");

		glUniformMatrix4fv(mloc, 1, GL_FALSE, &model);
		glUniformMatrix4fv(ploc, 1, GL_FALSE, &renderer->ortho);
		glUniform1f(aloc, 1.0f);
		glUniform3fv(cloc, 1, &color);

		glBindVertexArray(vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, NULL);

		endDraw(&dw);
	}

	glDeleteBuffers(1, &ibo);
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
	glDeleteProgram(shader);
	destroyRenderer(renderer);
	quitWindow(&dw);

	return 0;
}

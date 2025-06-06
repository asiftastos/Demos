#pragma once

typedef struct DMShaderInfo {
	const char* name;
	const char* vertexShaderFile;
	const char* fragmentShaderFile;
}DMShaderInfo;

typedef struct DMShader {
	GLuint id;
}DMShader;

char* loadFileText(const char* filename, int *length);
void freeText(char* text);

unsigned int loadShader(const char* filename, int shaderType);
unsigned int createProgram(GLuint vsh, GLuint fsh, const char* name);

DMShader* newShader(DMShaderInfo* shaderInfo);
void deleteShader(DMShader* shader);

//passing NULL will unbind any shader program previously bound
void Use(DMShader* shader);

#ifdef DM_ASSETS_IMPLEMENTATION

#include <stdlib.h>
#include <stdio.h>

#ifndef GLAD_GL_IMPLEMENTATION
#include "demos/opengl/glad.h"
#endif // GLAD_GL_IMPLEMENTATION

char* loadFileText(const char* filename, int *length)
{
	char* text = NULL;

	if (filename != NULL)
	{
		FILE* file = fopen(filename, "rt");

		if (file != NULL)
		{
			fseek(file, 0, SEEK_END);
			int size = ftell(file);
			fseek(file, 0, SEEK_SET);

			if (size > 0)
			{
				text = (char*)calloc((size + 1), sizeof(char));
				unsigned int count = (unsigned int)fread(text, sizeof(char), size, file);

				if (count < (unsigned int)size)
				{
					text = realloc(text, count + 1);
					*length = count;
				}
				else
				{
					*length = size;
				}

				text[count] = '\0';
			}

			fclose(file);
		}
	}

	return text;
}

void freeText(char* text)
{
	if (text)
	{
		free(text);
		text = NULL;
	}
}

unsigned int loadShader(const char* filename, int shaderType)
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

unsigned int createProgram(GLuint vsh, GLuint fsh, const char* name)
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

DMShader* newShader(DMShaderInfo* shaderInfo) {
	if (shaderInfo == NULL) {
		SDL_Log("ShaderInfo struct must be provided\n");
		return NULL;
	}

	unsigned int vs = loadShader(shaderInfo->vertexShaderFile, GL_VERTEX_SHADER);
	unsigned int fs = loadShader(shaderInfo->fragmentShaderFile, GL_FRAGMENT_SHADER);

	DMShader* s = (DMShader*)malloc(sizeof(DMShader));

	if (s) {
		s->id = createProgram(vs, fs, shaderInfo->name);

		if(s->id > 0)
			return s;
	}

	SDL_Log("Failed to create shader program %s\n", shaderInfo->name);

	return NULL;
}

void deleteShader(DMShader* shader) {
	if (shader) {
		glDeleteProgram(shader->id);
		free(shader);
	}
}

void Use(DMShader* shader)
{
	GLuint id = 0;

	if (shader)
		id = shader->id;

	glUseProgram(id);
}

#endif // DM_ASSETS_IMPLEMENTATION

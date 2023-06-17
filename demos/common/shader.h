#ifndef THAUM_SHADER_HEADER_H
#define THAUM_SHADER_HEADER_H

#include <stdlib.h>
#include "cglm/cglm.h"

typedef struct Shader Shader;

Shader* thmShaderCreate(const char* vspath, const char* fspath);
void thmShaderDestroy(Shader* s);
void thmShaderUse(Shader* s);
void thmShaderSetupUniforms(Shader* s, size_t n, const char** names);
void thmShaderUploadMatrix(Shader* s, const char* name, mat4 m);
void thmShaderUploadVec3(Shader* s, const char* name, vec3 v);
void thmShaderUploadFloat(Shader* s, const char* name, float f);

#endif // !THAUM_SHADER_HEADER_H

#ifdef THAUM_SHADER_IMPLEMENTATION

#include <stdio.h>
#include <assert.h>
#include "containers/hashmap.h"
#include "glad.h"

typedef struct Shader
{
    unsigned int program;
    unsigned int vshader;
    unsigned int fshader;
    map_int_t uniforms;
}Shader;

static GLuint thmShaderCompile(const char* path, unsigned int shaderType)
{
    GLuint s = glCreateShader(shaderType);
    FILE* f = fopen(path, "r");
    if (f == NULL)
    {
        printf("Cannot open shader %s", path);
        return 0;
    }

    char* text;
    long len;
    fseek(f, 0, SEEK_END);
    len = ftell(f);
    assert(len > 0);
    fseek(f, 0, SEEK_SET);
    text = calloc(1, len);
    assert(text != NULL);
    fread(text, 1, len, f);
    assert(strlen(text) > 0);
    fclose(f);

    glShaderSource(s, 1, (const char**)&text, NULL);
    glCompileShader(s);
    free(text);
    int compile_error = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &compile_error);
    if (compile_error == 0)
    {
        char log[1024];
        int i;
        glGetShaderInfoLog(s, 1024, &i, log);
        printf("Shader compile error: %s\n", log);
        return 0;
    }

    return s;
}

Shader* thmShaderCreate(const char* vspath, const char* fspath)
{
    Shader* s = (Shader*)calloc(1, sizeof(Shader));
    assert(s);
    s->vshader = thmShaderCompile(vspath, GL_VERTEX_SHADER);
    s->fshader = thmShaderCompile(fspath, GL_FRAGMENT_SHADER);

    s->program = glCreateProgram();
    glAttachShader(s->program, s->vshader);
    glAttachShader(s->program, s->fshader);
    glLinkProgram(s->program);
    int compile_error = 0;
    glGetProgramiv(s->program, GL_LINK_STATUS, &compile_error);
    if (compile_error == GL_FALSE)
    {
        char log[1024];
        int i;
        glGetProgramInfoLog(s->program, 1024, &i, log);
        printf("Shader link error: %s\n", log);
        return 0;
    }

    map_init(&s->uniforms);

    return s;
}

void thmShaderDestroy(Shader* s)
{
    if (s)
    {
        map_deinit(&s->uniforms);
        glDeleteProgram(s->program);
        free((void*)s);
        s = NULL;
    }
}

void thmShaderUse(Shader* s)
{
    glUseProgram(s->program);
}

void thmShaderSetupUniforms(Shader* s, size_t n, const char** names)
{
    if (names != NULL && n > 0)
    {
        for (int i = 0; i < n; i++)
        {
            int l = glGetUniformLocation(s->program, names[i]);
            map_set(&s->uniforms, names[i], l);
        }
    }
}

void thmShaderUploadMatrix(Shader* s, const char* name, mat4 m)
{
    int l = *(map_get(&s->uniforms, name));
    glUniformMatrix4fv(l, 1, GL_FALSE, m);
}

void thmShaderUploadVec3(Shader* s, const char* name, vec3 v)
{
    int l = *(map_get(&s->uniforms, name));
    glUniform3fv(l, 1, v);
}

void thmShaderUploadFloat(Shader* s, const char* name, float f)
{
    int l = *(map_get(&s->uniforms, name));
    glUniform1f(l, f);
}

#endif // THAUM_SHADER_IMPLEMENTATION

#ifndef THAUM_TEXTURE_HEADER_H
#define THAUM_TEXTURE_HEADER_H

#include <stdbool.h>

typedef struct TextureParams
{
    int wrapS;
    int wrapT;
    int minFilter;
    int magFilter;
    int format;
    bool mipmaps;
    int width;
    int height;
    int pixelStore;
    bool freeData;
    char* path;
    unsigned char* data; //if path is NULL this will be used to create the texture.should not be NULL too.will be freed
}TextureParams;

typedef struct Texture Texture;

Texture* thmTextureCreate(TextureParams params);
void thmTextureDestroy(Texture* texture);
void thmTextureSet(Texture* texture);

#endif // !THAUM_TEXTURE_HEADER_H

#ifdef THAUM_TEXTURE_IMPLEMENTATION

#include <stdlib.h>
#include <assert.h>
#include "glad.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

typedef struct Texture
{
    unsigned int handle;
    int width;
    int height;
    int components;
}Texture;

Texture* thmTextureCreate(TextureParams params)
{
    Texture* t = calloc(1, sizeof(Texture));
    assert(t != NULL);

    if (params.path != NULL)
        params.data = stbi_load(params.path, &params.width, &params.height, &t->components, 0);

    assert(params.data != NULL);
    t->width = params.width;
    t->height = params.height;

    glGenTextures(1, &t->handle);
    glBindTexture(GL_TEXTURE_2D, t->handle);

    if (params.pixelStore > 0)
        glPixelStorei(GL_UNPACK_ALIGNMENT, params.pixelStore);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, params.wrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, params.wrapT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, params.minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, params.magFilter);
    glTexImage2D(GL_TEXTURE_2D, 0, params.format, t->width, t->height, 0, params.format, GL_UNSIGNED_BYTE, params.data);
    if (params.mipmaps)
        glGenerateMipmap(GL_TEXTURE_2D);

    if (params.freeData)
        stbi_image_free(params.data);

    return t;
}

void thmTextureDestroy(Texture* texture)
{
    if (texture)
    {
        glDeleteTextures(1, &texture->handle);
        free(texture);
    }
}

void thmTextureSet(Texture* texture)
{
    glBindTexture(GL_TEXTURE_2D, texture->handle);
}

#endif // THAUM_TEXTURE_IMPLEMENTATION

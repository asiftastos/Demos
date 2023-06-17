#ifndef THAUM_VBUFFER_HEADER_H
#define THAUM_VBUFFER_HEADER_H

typedef struct VertexBuffer
{
    unsigned int id;
    int bufferType;
    int bufferUsage;
    unsigned int sizeInBytes;
    unsigned int elementCount;  //pass this to draw opengl funcs
}VertexBuffer;

VertexBuffer* thmVBufferCreate(int type, int usage);
void thmVBufferDestroy(VertexBuffer* vBuffer);
void thmVBufferSet(VertexBuffer* vBuffer);
void thmVBufferData(VertexBuffer* vBuffer, size_t elementSize, size_t count, void* data);

#endif // !THAUM_VBUFFER_HEADER_H

#ifdef THAUM_VBUFFER_IMPLEMENTATION

#include "stdlib.h"
#include "assert.h"
#include "glad.h"

VertexBuffer* thmVBufferCreate(int type, int usage)
{
    VertexBuffer* vb = calloc(1, sizeof(VertexBuffer));
    assert(vb != NULL);

    glGenBuffers(1, &vb->id);

    vb->bufferType = type;
    vb->bufferUsage = usage;
    vb->sizeInBytes = 0;
    vb->elementCount = 0;

    return vb;
}

void thmVBufferDestroy(VertexBuffer* vBuffer)
{
    if (vBuffer != NULL)
    {
        glDeleteBuffers(1, &vBuffer->id);
        free(vBuffer);
    }
}

void thmVBufferSet(VertexBuffer* vBuffer)
{
    glBindBuffer(vBuffer->bufferType, vBuffer->id);
}

void thmVBufferData(VertexBuffer* vBuffer, size_t elementSize, size_t count, void* data)
{
    vBuffer->sizeInBytes = elementSize * count;
    vBuffer->elementCount = count; // vBuffer->sizeInBytes / elementSize;
    glBufferData(vBuffer->bufferType, vBuffer->sizeInBytes, data, vBuffer->bufferUsage);
}

#endif // THAUM_VBUFFER_IMPLEMENTATION

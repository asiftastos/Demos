#ifndef THAUM_VARRAY_HEADER_H
#define THAUM_VARRAY_HEADER_H

#include "containers/vector.h"

typedef struct VertexAttribute
{
    int index;
    int elementCount;
    int stride;
    int offset;
    int elementType; //common GL_FLOAT
}VertexAttribute;

typedef vec_t(VertexAttribute) vec_VertexAttribute_t;

typedef struct VertexArray
{
    unsigned int id;
    vec_VertexAttribute_t attributes;
}VertexArray;

VertexArray* thmVArrayCreate(size_t attrCount, VertexAttribute* attrs);
void thmVArrayDestroy(VertexArray* vArray);
void thmVArraySet(VertexArray* vArray);
void thmVArrayAttributesSet(VertexArray* vArray);

#endif // !THAUM_VARRAY_HEADER_H

#ifdef THAUM_VARRAY_IMPLEMENTATION

#include "stdlib.h"
#include "glad.h"

VertexArray* thmVArrayCreate(size_t attrCount, VertexAttribute* attrs)
{
    VertexArray* va = calloc(1, sizeof(VertexArray));
    assert(va != NULL);

    glGenVertexArrays(1, &va->id);

    vec_init(&va->attributes);

    if (attrs != NULL && attrCount > 0)
    {
        vec_pusharr(&va->attributes, attrs, attrCount);
    }

    return va;
}

void thmVArrayDestroy(VertexArray* vArray)
{
    if (vArray != NULL)
    {
        vec_deinit(&vArray->attributes);
        glDeleteVertexArrays(1, &vArray->id);
        free(vArray);
    }
}

void thmVArraySet(VertexArray* vArray)
{
    glBindVertexArray(vArray->id);
}

void thmVArrayAttributesSet(VertexArray* vArray)
{
    int it = 0;
    VertexAttribute* va;
    vec_foreach_ptr(&vArray->attributes, va, it)
    {
        glVertexAttribPointer(va->index, va->elementCount, va->elementType, false, va->stride, (void*)va->offset);
        glEnableVertexAttribArray(va->index);
    }
}

#endif // THAUM_VARRAY_IMPLEMENTATION

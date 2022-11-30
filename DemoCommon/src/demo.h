#ifndef DEMO_COMMON_HEADER_H
#define DEMO_COMMON_HEADER_H

#include <stdbool.h>
#include "cglm/cglm.h"

typedef void (*DemoFunc)();

typedef struct Demo Demo;

typedef struct DemoParams
{
	vec2 windowSize;
	bool fullscreen;
	DemoFunc fInit;
	DemoFunc fUpdate;
	DemoFunc fRender;
	DemoFunc fDestroy;
	char title[128];
}DemoParams;

Demo* DemoCreate(DemoParams params);
void DemoRun();
void DemoTerminate();

#endif // !DEMO_COMMON_HEADER_H

#ifdef DEMO_COMMON_IMPLEMENTATION

#include <stdlib.h>
#include <assert.h>

typedef struct Demo
{
	DemoParams params;
}Demo;

Demo* DemoCreate(DemoParams params)
{
	Demo* d = (Demo*)malloc(sizeof(Demo));
	assert(d);

	d->params = params;

	return d;
}

void DemoRun()
{

}

void DemoTerminate()
{

}

#endif // DEMO_COMMON_IMPLEMENTATION

#ifndef DEMO_COMMON_HEADER_H
#define DEMO_COMMON_HEADER_H

#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include "HandmadeMath.h"

typedef void (*DemoFunc)();

typedef struct Demo Demo;

typedef struct DemoParams
{
	HMM_Vec2 prefferedWindowSize;
	bool fullscreen;
	DemoFunc fInit;
	DemoFunc fUpdate;
	DemoFunc fRender;
	DemoFunc fDestroy;
	char title[128];
}DemoParams;

Demo* DemoCreate(DemoParams* params);
void DemoRun(Demo* d);

#endif // !DEMO_COMMON_HEADER_H

#ifdef DEMO_COMMON_IMPLEMENTATION

typedef struct Demo
{
	DemoParams* params;
	bool running;
}Demo;

Demo* DemoCreate(DemoParams* params)
{
	Demo* d = (Demo*)malloc(sizeof(Demo));
	assert(d);

	d->params = params;
	d->running = false;

	return d;
}

static void DemoTerminate(Demo* d)
{
	d->params->fDestroy();
}

void DemoRun(Demo* d)
{
	d->params->fInit();

	while (d->running)
	{
		d->params->fUpdate();
		d->params->fRender();
	}

	DemoTerminate(d);
}

#endif // DEMO_COMMON_IMPLEMENTATION

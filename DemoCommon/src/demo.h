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

Demo* DemoCreate(DemoParams* params);
void DemoRun(Demo* d);
void DemoTerminate(Demo* d);

#endif // !DEMO_COMMON_HEADER_H

#ifdef DEMO_COMMON_IMPLEMENTATION

#include <stdlib.h>
#include <assert.h>
#define THAUM_WINDOW_IMPLEMENTATION
#include "src/thmWindow.h"

typedef struct Demo
{
	DemoParams* params;
	ThmWindowContext* thmWindowContext;
}Demo;

Demo* DemoCreate(DemoParams* params)
{
	Demo* d = (Demo*)malloc(sizeof(Demo));
	assert(d);

	d->params = params;

	ThmWindowParams winParams;
	winParams.fullscreen = params->fullscreen;
	glm_vec2(params->windowSize, winParams.windowSize);
	strcpy(winParams.title, params->title);
	d->thmWindowContext = thmCreateContext(winParams);

	return d;
}

void DemoRun(Demo* d)
{

}

void DemoTerminate(Demo* d)
{
	d->params->fDestroy();

	if (d->thmWindowContext)
		thmDestroyContext(d->thmWindowContext);
}

#endif // DEMO_COMMON_IMPLEMENTATION

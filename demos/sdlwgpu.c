/*
*	SDL WGPU
*/

#include <stdio.h>
#include <SDL_syswm.h>
#define DM_WINDOW_IMPLEMENTATION
#include <dmWindow.h>

DmWindow dw;

#include <wgpu.h>

typedef struct DmWGPU {
	WGPUInstance instance;
	WGPUSurface surface;
	WGPUAdapter adapter;
}DmWGPU;

DmWGPU dwgpu;

void onAdapterRequestEnded(WGPURequestAdapterStatus status, WGPUAdapter adapter, char const* message, void* userdata)
{
	if (status == WGPURequestAdapterStatus_Success)
	{
		dwgpu.adapter = adapter;
	}
	else
	{
		printf("Failed to get WGPU adapter: %s\n", message);
	}
}

void InitWGPU()
{
	WGPUInstanceDescriptor instanceDescriptor = {
		.nextInChain = NULL,
	};
	dwgpu.instance = wgpuCreateInstance(&instanceDescriptor);

	if (!dwgpu.instance)
	{
		printf("Failed to create WGPU instance\n");
		return;
	}

	printf("WGPU Instance created at %p\n", dwgpu.instance);

	//surface
	SDL_SysWMinfo swmInfo;
	SDL_GetWindowWMInfo(dw.window, &swmInfo);
	WGPUSurfaceDescriptor surfDesc = {
		.label = NULL,
		.nextInChain = (const WGPUChainedStruct*)&(WGPUSurfaceDescriptorFromWindowsHWND) {
			.chain = (WGPUChainedStruct){ .next = NULL, .sType = WGPUSType_SurfaceDescriptorFromWindowsHWND, },
			.hinstance = swmInfo.info.win.hinstance,
			.hwnd = swmInfo.info.win.window,
		},
	};
	dwgpu.surface = wgpuInstanceCreateSurface(dwgpu.instance, &surfDesc);

	//request adapter
	WGPURequestAdapterOptions adapterOptions = {
		.nextInChain = NULL,
		.compatibleSurface = dwgpu.surface,
	};
	wgpuInstanceRequestAdapter(dwgpu.instance, &adapterOptions, onAdapterRequestEnded, NULL);

	printf("Got WGPU adapter at %p\n", dwgpu.adapter);
}

void handleKeyboard(SDL_KeyboardEvent *ev)
{
	if (ev->keysym.sym == SDLK_ESCAPE)
		dw.running = false;
	if (ev->keysym.sym == SDLK_F1)
		GrabMouse(&dw);
	if (ev->keysym.sym == SDLK_F2)
		ReleaseMouse(&dw);
}

int main(int argc, const char* argv[])
{
	DmWindowParams dparams = { 
		.width = 1024, 
		.height = 768, 
		.api = NOAPI, 
		.title = "SDL WGPU" 
	};

	if (InitWindow(&dparams, &dw) > 0)
		return 1;

	dw.keyboardHandler = handleKeyboard;

	InitWGPU();

	dw.running = true;

	SDL_Event e = { 0 };
	while (dw.running)
	{
		ProcessEvents(&dw, &e);
	}

	//wgpu
	wgpuAdapterDrop(dwgpu.adapter);
	wgpuInstanceDrop(dwgpu.instance);

	//sdl
	QuitWindow(&dw);

	return 0;
}
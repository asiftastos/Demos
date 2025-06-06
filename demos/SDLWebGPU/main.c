/*
*	SDL WebGPU native
* 
* 
*/

#define HINSTANCE void*
#define HWND void*

#include <stdio.h>
#define DM_WINDOW_IMPLEMENTATION
#include "demos/common/dmWindow.h"

DmWindow dw;

const static int windowWidth = 800;
const static int windowHeight = 600;

#define DM_WGPU_IMPLEMENTATION
#include "demos/wgpu/dmgpu.h"
DmWGPU dwgpu;


void handleKeyboard(SDL_KeyboardEvent* ev)
{
	if (ev->key == SDLK_ESCAPE)
		dw.running = false;
	if (ev->key == SDLK_F1)
		GrabMouse(&dw);
	if (ev->key == SDLK_F2)
		ReleaseMouse(&dw);
}

int main(int argc, const char* argv[])
{
	DmWindowParams dparams = {
		.width = windowWidth,
		.height = windowHeight,
		.api = NOAPI,
		.title = "WebGPU native"
	};

	if (InitWindow(&dparams, &dw) > 0)
		return 1;

	dw.keyboardHandler = handleKeyboard;


	//SDL_SysWMinfo swmInfo;
	//SDL_VERSION(&swmInfo.version);
	//SDL_GetWindowWMInfo(dw.window, &swmInfo);
	SDL_PropertiesID winProps = SDL_GetWindowProperties(dw.window);
	HINSTANCE hinst = SDL_GetPointerProperty(winProps, SDL_PROP_WINDOW_WIN32_INSTANCE_POINTER, NULL);
	HWND hwnd = SDL_GetPointerProperty(winProps, SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);


	WgpuParams gpuParams = {
		.windowWidth = windowWidth,
		.windowHeight = windowHeight,
		.hInstance = hinst,
		.hwnd = hwnd,
	};

	InitWGPU(gpuParams, &dwgpu);

	dw.running = true;

	SDL_Event e = { 0 };
	while (dw.running)
	{
		ProcessEvents(&dw, &e);

		WGPUSurfaceTexture surfTexture;
		wgpuSurfaceGetCurrentTexture(dwgpu.surface, &surfTexture);

		WGPUTextureView nextTexture = wgpuTextureCreateView(surfTexture.texture, NULL);

		WGPUCommandEncoderDescriptor cmdEncoderDesc = {
			.nextInChain = NULL,
			.label = "Command Encoder"
		};
		WGPUCommandEncoder cmdEncoder = wgpuDeviceCreateCommandEncoder(dwgpu.device, &cmdEncoderDesc);

		WGPURenderPassColorAttachment renderPassColorAttach = {
			.view = nextTexture,
			.resolveTarget = NULL,
			.loadOp = WGPULoadOp_Clear,
			.storeOp = WGPUStoreOp_Store,
			.clearValue = (WGPUColor){0.2, 0.2, 0.2, 1.0},
		};

		WGPURenderPassDescriptor renderPassDesc = { 
			.colorAttachmentCount = 1,
			.colorAttachments = &renderPassColorAttach,
			.depthStencilAttachment = NULL,
			.timestampWrites = NULL,
			.nextInChain = NULL,
		};

		WGPURenderPassEncoder renderPass = wgpuCommandEncoderBeginRenderPass(cmdEncoder, &renderPassDesc);
		wgpuRenderPassEncoderEnd(renderPass);

		wgpuTextureViewRelease(nextTexture);

		WGPUCommandBufferDescriptor cmdBufferDesc = {
			.nextInChain = NULL,
			.label = "Command Buffer"
		};

		WGPUCommandBuffer cmdBuffer = wgpuCommandEncoderFinish(cmdEncoder, &cmdBufferDesc);
		wgpuCommandEncoderRelease(cmdEncoder);
		wgpuQueueSubmit(dwgpu.queue, 1, &cmdBuffer);
		wgpuCommandBufferRelease(cmdBuffer);

		wgpuSurfacePresent(dwgpu.surface);
	}

	//wgpu
	ReleaseWGPU(&dwgpu);

	//sdl
	QuitWindow(&dw);

	return 0;
}

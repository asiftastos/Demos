/*
*	SDL WebGPU native
* 
* TODO: fix swapchain not created because of invalid surface format
*/

#include <stdio.h>
#include "SDL2/SDL_syswm.h"
#define DM_WINDOW_IMPLEMENTATION
#include "demos/common/dmWindow.h"

DmWindow dw;

#include "WGPU/wgpu.h"

typedef struct DmWGPU {
	WGPUInstance instance;
	WGPUSurface surface;
	WGPUAdapter adapter;
	WGPUDevice device;
	WGPUQueue queue;
	WGPUSwapChain swapchain;
}DmWGPU;

DmWGPU dwgpu;

const char* GetBackendType(WGPUBackendType backend)
{
	switch (backend)
	{
	case WGPUBackendType_WebGPU:
		return "WebGPU";
	case WGPUBackendType_D3D11:
		return "Direct3D11";
	case WGPUBackendType_D3D12:
		return "Direct3D12";
	case WGPUBackendType_Vulkan:
		return "Vulkan";
	case WGPUBackendType_OpenGL:
		return "OpenGL";
	default:
		return "NULL";
	}
}

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

void onDeviceRequest(WGPURequestDeviceStatus status, WGPUDevice device, char const* message, void* userdata)
{
	if (status == WGPURequestAdapterStatus_Success)
	{
		dwgpu.device = device;
	}
	else
	{
		printf("Request device error: %s\n", message);
	}
}

void onDeviceError(WGPUErrorType type, char const* message, void* userdata)
{
	printf("Device error: %s\n", message);
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
			.chain = (WGPUChainedStruct){.next = NULL, .sType = WGPUSType_SurfaceDescriptorFromWindowsHWND, },
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

	//adapter properties
	WGPUAdapterProperties adapterProperties = { 0 };
	wgpuAdapterGetProperties(dwgpu.adapter, &adapterProperties);

	printf("Name: %s\n", adapterProperties.name);
	printf("Vendor name: %s\n", adapterProperties.vendorName);
	printf("Backend: %s\n", GetBackendType(adapterProperties.backendType));

	//request device
	WGPUDeviceDescriptor devDesc = {
		.nextInChain = NULL,
		.label = "My device",
		.requiredFeaturesCount = 0,
		.requiredLimits = NULL,
		.defaultQueue.nextInChain = NULL,
		.defaultQueue.label = "Default Queue",
	};
	wgpuAdapterRequestDevice(dwgpu.adapter, &devDesc, onDeviceRequest, NULL);

	printf("Got WGPU device at %p\n", dwgpu.device);

	wgpuDeviceSetUncapturedErrorCallback(dwgpu.device, onDeviceError, NULL);

	//get the main queue
	dwgpu.queue = wgpuDeviceGetQueue(dwgpu.device);

	//swapchain
	WGPUTextureFormat swapchainFormat = wgpuSurfaceGetPreferredFormat(dwgpu.surface, dwgpu.adapter);

	WGPUSwapChainDescriptor swapDesc = {
		.nextInChain = NULL,
		.width = 1024,
		.height = 768,
		.format = swapchainFormat,
		.usage = WGPUTextureUsage_RenderAttachment,
		.presentMode = WGPUPresentMode_Fifo,
	};
	dwgpu.swapchain = wgpuDeviceCreateSwapChain(dwgpu.device, dwgpu.surface, &swapDesc);

	printf("Got WGPU swapchain at %p\n", dwgpu.swapchain);
}

void handleKeyboard(SDL_KeyboardEvent* ev)
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
		.title = "WebGPU native"
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

		WGPUTextureView nextTexture = wgpuSwapChainGetCurrentTextureView(dwgpu.swapchain);

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
			.clearValue = (WGPUColor){0.3f, 0.3f, 0.3f, 1.0f},
		};

		WGPURenderPassDescriptor renderPassDesc = { 
			.colorAttachmentCount = 1,
			.colorAttachments = &renderPassColorAttach,
			.depthStencilAttachment = NULL,
			.timestampWriteCount = 0,
			.timestampWrites = NULL,
			.nextInChain = NULL,
		};

		WGPURenderPassEncoder renderPass = wgpuCommandEncoderBeginRenderPass(cmdEncoder, &renderPassDesc);
		wgpuRenderPassEncoderEnd(renderPass);

		wgpuTextureViewDrop(nextTexture);

		WGPUCommandBufferDescriptor cmdBufferDesc = {
			.nextInChain = NULL,
			.label = "Command Buffer"
		};

		WGPUCommandBuffer cmdBuffer = wgpuCommandEncoderFinish(cmdEncoder, &cmdBufferDesc);
		wgpuQueueSubmit(dwgpu.queue, 1, &cmdBuffer);

		wgpuSwapChainPresent(dwgpu.swapchain);
	}

	//wgpu
	wgpuSwapChainDrop(dwgpu.swapchain);
	wgpuDeviceDrop(dwgpu.device);
	wgpuSurfaceDrop(dwgpu.surface);
	wgpuAdapterDrop(dwgpu.adapter);
	wgpuInstanceDrop(dwgpu.instance);

	//sdl
	QuitWindow(&dw);

	return 0;
}

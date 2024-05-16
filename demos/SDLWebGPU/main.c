/*
*	SDL WebGPU native
* 
* 
*/

#include <stdio.h>
#define DM_WINDOW_IMPLEMENTATION
#include "demos/common/dmWindow.h"
#include "SDL2/SDL_syswm.h"

DmWindow dw;

#include "WGPU/wgpu.h"

const static int windowWidth = 800;
const static int windowHeight = 600;

typedef struct DmWGPU {
	WGPUInstance instance;
	WGPUSurface surface;
	WGPUAdapter adapter;
	WGPUDevice device;
	WGPUQueue queue;
	WGPUSurfaceCapabilities surfCaps;
	WGPUSurfaceConfiguration surfConfig;
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
	SDL_VERSION(&swmInfo.version);
	SDL_GetWindowWMInfo(dw.window, &swmInfo);
	HINSTANCE hinst = swmInfo.info.win.hinstance;
	HWND hwnd = swmInfo.info.win.window;
	WGPUSurfaceDescriptor surfDesc = {
		.label = NULL,
		.nextInChain = (const WGPUChainedStruct*)&(WGPUSurfaceDescriptorFromWindowsHWND) {
			.chain = (WGPUChainedStruct){.next = NULL, .sType = WGPUSType_SurfaceDescriptorFromWindowsHWND, },
			.hinstance = hinst,
			.hwnd = hwnd,
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
		.requiredFeatureCount = 0,
		.requiredLimits = NULL,
		.defaultQueue.nextInChain = NULL,
		.defaultQueue.label = "Default Queue",
	};
	wgpuAdapterRequestDevice(dwgpu.adapter, &devDesc, onDeviceRequest, NULL);

	printf("Got WGPU device at %p\n", dwgpu.device);

	wgpuDeviceSetUncapturedErrorCallback(dwgpu.device, onDeviceError, NULL);

	//get the main queue
	dwgpu.queue = wgpuDeviceGetQueue(dwgpu.device);

	//surface configuration, replaces swapchain
	WGPUTextureFormat surfFormat = wgpuSurfaceGetPreferredFormat(dwgpu.surface, dwgpu.adapter);
	wgpuSurfaceGetCapabilities(dwgpu.surface, dwgpu.adapter, &dwgpu.surfCaps);

	dwgpu.surfConfig.device = dwgpu.device;
	dwgpu.surfConfig.format = surfFormat;
	dwgpu.surfConfig.width = windowWidth;
	dwgpu.surfConfig.height = windowHeight;
	dwgpu.surfConfig.usage = WGPUTextureUsage_RenderAttachment;
	dwgpu.surfConfig.presentMode = WGPUPresentMode_Fifo;

	wgpuSurfaceConfigure(dwgpu.surface, &dwgpu.surfConfig);
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
		.width = windowWidth,
		.height = windowHeight,
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

	//wgpu release
	wgpuQueueRelease(dwgpu.queue);
	wgpuDeviceRelease(dwgpu.device);
	wgpuSurfaceRelease(dwgpu.surface);
	wgpuAdapterRelease(dwgpu.adapter);
	wgpuInstanceRelease(dwgpu.instance);

	//sdl
	QuitWindow(&dw);

	return 0;
}

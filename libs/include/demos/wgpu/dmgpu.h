#pragma once

#include "WGPU/wgpu.h"

typedef struct WgpuParams {
	int windowWidth;
	int windowHeight;
	void* hInstance;
	void* hwnd;
}WgpuParams;

typedef struct DmWGPU {
	WGPUInstance instance;
	WGPUSurface surface;
	WGPUAdapter adapter;
	WGPUDevice device;
	WGPUQueue queue;
	WGPUSurfaceCapabilities surfCaps;
	WGPUSurfaceConfiguration surfConfig;
}DmWGPU;

const char* GetBackendType(WGPUBackendType backend);

int InitWGPU(WgpuParams gpuParams, DmWGPU* wgpuContext);
void ReleaseWGPU(DmWGPU* wgpuContext);

#ifdef  DM_WGPU_IMPLEMENTATION

static void onAdapterRequestEnded(WGPURequestAdapterStatus status, WGPUAdapter adapter, char const* message, void* userdata)
{
	if (status == WGPURequestAdapterStatus_Success)
	{
		DmWGPU* context = (DmWGPU*)userdata;
		context->adapter = adapter;
	}
	else
	{
		printf("Failed to get WGPU adapter: %s\n", message);
	}
}

static void onDeviceRequest(WGPURequestDeviceStatus status, WGPUDevice device, char const* message, void* userdata)
{
	if (status == WGPURequestAdapterStatus_Success)
	{
		DmWGPU* context = (DmWGPU*)userdata;
		context->device = device;
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

int InitWGPU(WgpuParams gpuParams, DmWGPU* wgpuContext)
{
	//create wgpu instance
	WGPUInstanceDescriptor instanceDescriptor = {
		.nextInChain = NULL,
	};
	wgpuContext->instance = wgpuCreateInstance(&instanceDescriptor);

	if (!wgpuContext->instance)
	{
		printf("Failed to create WGPU instance\n");
		return 1;
	}
	printf("WGPU Instance created at %p\n", &wgpuContext->instance);


	//surface
	WGPUSurfaceDescriptor surfDesc = {
		.label = NULL,
		.nextInChain = (const WGPUChainedStruct*)&(WGPUSurfaceDescriptorFromWindowsHWND) {
			.chain = (WGPUChainedStruct){.next = NULL, .sType = WGPUSType_SurfaceDescriptorFromWindowsHWND, },
			.hinstance = gpuParams.hInstance,
			.hwnd = gpuParams.hwnd,
		},
	};
	wgpuContext->surface = wgpuInstanceCreateSurface(wgpuContext->instance, &surfDesc);

	//request adapter
	WGPURequestAdapterOptions adapterOptions = {
		.nextInChain = NULL,
		.compatibleSurface = wgpuContext->surface,
	};
	wgpuInstanceRequestAdapter(wgpuContext->instance, &adapterOptions, onAdapterRequestEnded, (void*)wgpuContext);

	printf("Got WGPU adapter at %p\n", wgpuContext->adapter);

	//adapter properties
	WGPUAdapterProperties adapterProperties = { 0 };
	wgpuAdapterGetProperties(wgpuContext->adapter, &adapterProperties);

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
	wgpuAdapterRequestDevice(wgpuContext->adapter, &devDesc, onDeviceRequest, (void*)wgpuContext);

	printf("Got WGPU device at %p\n", wgpuContext->device);

	wgpuDeviceSetUncapturedErrorCallback(wgpuContext->device, onDeviceError, NULL);


	//get the main queue
	wgpuContext->queue = wgpuDeviceGetQueue(wgpuContext->device);


	//surface configuration, replaces swapchain
	WGPUTextureFormat surfFormat = wgpuSurfaceGetPreferredFormat(wgpuContext->surface, wgpuContext->adapter);
	wgpuSurfaceGetCapabilities(wgpuContext->surface, wgpuContext->adapter, &wgpuContext->surfCaps);

	wgpuContext->surfConfig.device = wgpuContext->device;
	wgpuContext->surfConfig.format = surfFormat;
	wgpuContext->surfConfig.width = windowWidth;
	wgpuContext->surfConfig.height = windowHeight;
	wgpuContext->surfConfig.usage = WGPUTextureUsage_RenderAttachment;
	wgpuContext->surfConfig.presentMode = WGPUPresentMode_Fifo;

	wgpuSurfaceConfigure(wgpuContext->surface, &wgpuContext->surfConfig);

	return 0;
}

void ReleaseWGPU(DmWGPU* wgpuContext)
{
	wgpuQueueRelease(wgpuContext->queue);
	wgpuDeviceRelease(wgpuContext->device);
	wgpuSurfaceRelease(wgpuContext->surface);
	wgpuAdapterRelease(wgpuContext->adapter);
	wgpuInstanceRelease(wgpuContext->instance);
}

#endif //  DM_WGPU_IMPLEMENTATION

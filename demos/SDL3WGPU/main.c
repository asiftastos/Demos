#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>

typedef struct App {
	SDL_Window* window;
	SDL_GPUDevice* device;
}App;

SDL_AppResult AppCreate(App** application) {
	*application = (App*)SDL_malloc(sizeof(App));

	if (*application == NULL) {
		return SDL_APP_FAILURE;
	}
}

void AppDelete(App* app) {
	if (app)
		SDL_free(app);
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv) {
	App* app = NULL;
	SDL_AppResult result = AppCreate(&app);
	if(result == SDL_APP_FAILURE){
		SDL_Log("Failed to create application\n");
		return result;
	}

	// create window
	app->window = SDL_CreateWindow("Hello SDL3 WGPU Triangle", 1280, 800, SDL_WINDOW_RESIZABLE);

	//create gpu device
	app->device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, NULL);
	SDL_ClaimWindowForGPUDevice(app->device, app->window);

	*appstate = (void*)app;

	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate) {
	App* app = (App*)appstate;

	//aquire the command buffer
	SDL_GPUCommandBuffer* cmdBuffer = SDL_AcquireGPUCommandBuffer(app->device);

	//get the swapchain texture
	SDL_GPUTexture* swapchainTexture;
	Uint32 width, height;
	SDL_WaitAndAcquireGPUSwapchainTexture(cmdBuffer, app->window, &swapchainTexture, &width, &height);

	//skip frame if no swapchain texture aquired
	if (swapchainTexture == NULL) {
		//always submit command buffer
		SDL_SubmitGPUCommandBuffer(cmdBuffer);
		return SDL_APP_CONTINUE;
	}

	//create the color target
	SDL_GPUColorTargetInfo colorTargetInfo = {
		.clear_color = {0.4f, 0.4f, 0.4f, 1.0f},
		.load_op = SDL_GPU_LOADOP_CLEAR,
		.store_op = SDL_GPU_STOREOP_STORE,
		.texture = swapchainTexture
	};

	//begin a render pass
	SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(cmdBuffer, &colorTargetInfo, 1, NULL);

	//draw here

	//end ther render pass
	SDL_EndGPURenderPass(renderPass);

	SDL_SubmitGPUCommandBuffer(cmdBuffer);

	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
	if (event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
		return SDL_APP_SUCCESS;
	}

	return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
	App* app = (App*)appstate;
	SDL_DestroyGPUDevice(app->device);
	SDL_DestroyWindow(app->window);
	AppDelete(app);
}
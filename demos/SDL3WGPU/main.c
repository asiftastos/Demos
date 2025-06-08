#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>

typedef struct Mesh {
	SDL_GPUBuffer* vertexBuffer;
	SDL_GPUTransferBuffer* transBuffer;
	SDL_GPUGraphicsPipeline* pipeLine;
}Mesh;

typedef struct App {
	SDL_Window* window;
	SDL_GPUDevice* device;
	Mesh* triangle;
}App;

typedef struct Vertex {
	float x, y, z;
	float r, g, b, a;
}Vertex;

static Vertex vertices[] = {
	{0.0f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f},
	{-0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f},
	{0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f}
};

//shaders
SDL_GPUShader* LoadShaderFromFile(SDL_GPUDevice* device, const char* filename, SDL_GPUShaderStage stage) {
	size_t codeSize;
	void* shaderCode = SDL_LoadFile(filename, &codeSize);

	SDL_GPUShaderCreateInfo shaderInfo = {
		.code = (Uint8*)shaderCode,
		.code_size = codeSize,
		.entrypoint = "main",
		.format = SDL_GPU_SHADERFORMAT_SPIRV,
		.stage = stage,
		.num_samplers = 0,
		.num_storage_buffers = 0,
		.num_storage_textures = 0,
		.num_uniform_buffers = 0
	};

	SDL_GPUShader* shader = SDL_CreateGPUShader(device, &shaderInfo);

	SDL_free(shaderCode);

	return shader;
}

//pipeline
void PipelineCreate(Mesh* mesh, SDL_GPUDevice* device, SDL_Window* window) {
	SDL_GPUShader* vs = LoadShaderFromFile(device, "assets/shaders/vertex.spv", SDL_GPU_SHADERSTAGE_VERTEX);
	SDL_GPUShader* fs = LoadShaderFromFile(device, "assets/shaders/fragment.spv", SDL_GPU_SHADERSTAGE_FRAGMENT);

	//describe the vertex buffers 
	SDL_GPUVertexBufferDescription vertexBufDesc[1];
	vertexBufDesc[0].slot = 0;
	vertexBufDesc[0].input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
	vertexBufDesc[0].instance_step_rate = 0;
	vertexBufDesc[0].pitch = sizeof(Vertex);

	//describe the vertex attributes
	SDL_GPUVertexAttribute vertexAttrs[2];

	//a_position attrinute in the vertex shader
	vertexAttrs[0].buffer_slot = 0;
	vertexAttrs[0].location = 0;
	vertexAttrs[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
	vertexAttrs[0].offset = 0;

	//a_color attrinute in the vertex shader
	vertexAttrs[1].buffer_slot = 0;
	vertexAttrs[1].location = 1;
	vertexAttrs[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
	vertexAttrs[1].offset = sizeof(float) * 3;

	//describe the color target
	SDL_GPUColorTargetDescription colorTargetDescs[1];
	colorTargetDescs[0] = (SDL_GPUColorTargetDescription){
		.format = SDL_GetGPUSwapchainTextureFormat(device, window)
	};

	SDL_GPUGraphicsPipelineCreateInfo pipelineInfo = {
		.vertex_shader = vs,
		.fragment_shader = fs,
		.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
		.vertex_input_state.num_vertex_buffers = 1,
		.vertex_input_state.vertex_buffer_descriptions = vertexBufDesc,
		.vertex_input_state.num_vertex_attributes = 2,
		.vertex_input_state.vertex_attributes = vertexAttrs,
		.target_info.num_color_targets = 1,
		.target_info.color_target_descriptions = colorTargetDescs
	};

	mesh->pipeLine = SDL_CreateGPUGraphicsPipeline(device, &pipelineInfo);

	//after creation of the pipeline we don't need them anymore
	SDL_ReleaseGPUShader(device, fs);
	SDL_ReleaseGPUShader(device, vs);
}

//App
SDL_AppResult AppCreate(App** application) {
	*application = (App*)SDL_malloc(sizeof(App));

	if (*application == NULL) {
		return SDL_APP_FAILURE;
	}

	return SDL_APP_CONTINUE;
}

void AppDelete(App* app) {
	if (app)
		SDL_free(app);
}

//Mesh
SDL_AppResult MeshCreate(Mesh** mesh, SDL_GPUDevice* device, SDL_GPUBufferCreateInfo* bufCreateInfo) {
	*mesh = (Mesh*)SDL_malloc(sizeof(Mesh));

	if (*mesh == NULL)
		return SDL_APP_FAILURE;

	(*mesh)->vertexBuffer = SDL_CreateGPUBuffer(device, bufCreateInfo);

	(*mesh)->transBuffer = NULL;

	return SDL_APP_CONTINUE;
}

void MeshDelete(Mesh* mesh, SDL_GPUDevice* device) {
	if (mesh == NULL)
		return;

	SDL_ReleaseGPUGraphicsPipeline(device, mesh->pipeLine);
	SDL_ReleaseGPUTransferBuffer(device, mesh->transBuffer);
	SDL_ReleaseGPUBuffer(device, mesh->vertexBuffer);

	SDL_free(mesh);
}

void MeshUpload(Mesh* mesh, SDL_GPUDevice* device, SDL_GPUTransferBufferCreateInfo* transCreateInfo) {
	if (transCreateInfo != NULL) {
		mesh->transBuffer = SDL_CreateGPUTransferBuffer(device, transCreateInfo);
	}

	//map the transferbuffer to a pointer and copy the data
	Vertex* data = (Vertex*)SDL_MapGPUTransferBuffer(device, mesh->transBuffer, false);
	SDL_memcpy(data, vertices, sizeof(vertices));
	SDL_UnmapGPUTransferBuffer(device, mesh->transBuffer);

	//start copy pass from transferbuffer to gpu vertexbuffer
	SDL_GPUCommandBuffer* cmdBuffer = SDL_AcquireGPUCommandBuffer(device);
	SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmdBuffer);

	//from
	SDL_GPUTransferBufferLocation location = {
		.transfer_buffer = mesh->transBuffer,
		.offset = 0
	};

	//to
	SDL_GPUBufferRegion region = {
		.buffer = mesh->vertexBuffer,
		.size = sizeof(vertices),
		.offset = 0
	};

	//upload and end pass
	SDL_UploadToGPUBuffer(copyPass, &location, &region, false);
	SDL_EndGPUCopyPass(copyPass);
	SDL_SubmitGPUCommandBuffer(cmdBuffer);
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

	SDL_GPUBufferCreateInfo bufCreateInfo = {
		.size = sizeof(vertices),
		.usage = SDL_GPU_BUFFERUSAGE_VERTEX
	};
	result = MeshCreate(&(app->triangle), app->device, &bufCreateInfo);
	
	if (result == SDL_APP_FAILURE) {
		SDL_Log("Failed to create mesh\n");
		return result;
	}

	SDL_GPUTransferBufferCreateInfo transCreateInfo = {
		.size = sizeof(vertices),
		.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD
	};
	MeshUpload(app->triangle, app->device, &transCreateInfo);

	//shaders and pipeline
	PipelineCreate(app->triangle, app->device, app->window);

	return result;
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
	//bind the pipeline
	SDL_BindGPUGraphicsPipeline(renderPass, app->triangle->pipeLine);

	//bind the vertex buffer to buffer slot 0
	SDL_GPUBufferBinding bufBindings[1];
	bufBindings[0].buffer = app->triangle->vertexBuffer;
	bufBindings[0].offset = 0;

	SDL_BindGPUVertexBuffers(renderPass, 0, bufBindings, 1);

	//draw call
	SDL_DrawGPUPrimitives(renderPass, 3, 1, 0, 0);

	//end ther render pass
	SDL_EndGPURenderPass(renderPass);

	SDL_SubmitGPUCommandBuffer(cmdBuffer);

	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
	if (event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
		return SDL_APP_SUCCESS;
	}

	if (event->type == SDL_EVENT_KEY_DOWN) {
		if (event->key.key == SDLK_ESCAPE)
			return SDL_APP_SUCCESS;
	}

	return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
	App* app = (App*)appstate;
	MeshDelete(app->triangle, app->device);
	SDL_DestroyGPUDevice(app->device);
	SDL_DestroyWindow(app->window);
	AppDelete(app);
}
#include "demo.h"
#include "demoui.h"

static Demo* d = NULL;
static Demoui* dui = NULL;

static GLuint quadVao;
static GLuint quadVbo;
static Shader* textureShader;
static mat4s model;
static mat4s proj;
static Texture* quadTexture;

static void render2D()
{
    textureSet(quadTexture);
    shaderUse(textureShader);
    shaderUploadMatrix(textureShader, "model", model);
    shaderUploadMatrix(textureShader, "proj", proj);
    glBindVertexArray(quadVao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}

void init()
{
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    dui = demouiInit(d);

    mat4s rotate = glms_rotate_y(GLMS_MAT4_IDENTITY, glm_rad(45.0f));
    mat4s trans = glms_translate_z(GLMS_MAT4_IDENTITY, 20.0f);
    model = glms_mat4_mul(trans, rotate);
    

    mat4s view;
    view = glms_lookat((vec3s){{0.0f, 0.0f, -40.0f}}, GLMS_VEC3_ZERO, GLMS_YUP);
    //model = glms_mat4_mul(view, model);
    
    //glm_ortho(0.0f, d->fbSize[0], 0.0f, d->fbSize[1], 0.1f, 10.0f, proj);
    proj = glms_perspective(glm_rad(60.0f), d->fbSize.x / d->fbSize.y, 0.1f, 1000.0f);
    proj = glms_mat4_mul(proj, view);

    quadTexture = textureCreate((TextureParams){
        .format = GL_RGB,
        .wrapS = GL_REPEAT,
        .wrapT = GL_REPEAT,
        .minFilter = GL_LINEAR,
        .magFilter = GL_LINEAR,
        .mipmaps = false,
        .path = "assets/textures/wall.jpg"
    });

    textureShader = shaderCreate("assets/shaders/demotext.vert", "assets/shaders/texture.frag");
    shaderUse(textureShader);
    shaderSetupUniforms(textureShader, 2, (const char*[]){"model", "proj"});
    glUseProgram(0);

    float quad[] = {
        0.0f, 0.0f,   0.0f,      0.0f, 0.0f,
        20.0f, 0.0f,  0.0f,      1.0f, 0.0f,
        0.0f, 20.0f,  0.0f,      0.0f, 1.0f,
        0.0f, 20.0f,  0.0f,      0.0f, 1.0f,
        20.0f, 0.0f,  0.0f,      1.0f, 0.0f,
        20.0f, 20.0f, 0.0f,      1.0f, 1.0f
    };

    glGenVertexArrays(1, &quadVao);
    glBindVertexArray(quadVao);
    glGenBuffers(1, &quadVbo);
    glBindBuffer(GL_ARRAY_BUFFER, quadVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 30, quad, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float) * 3));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    printf("Demo texturing initialized\n");
}

void terminate()
{
    textureDestroy(quadTexture);

    demouiTerminate();
    printf("Demo texturing terminated\n");
}

void update()
{
    if(d->keys[GLFW_KEY_ESCAPE].pressed)
        glfwSetWindowShouldClose(d->window, GLFW_TRUE);
    
    if(d->keys[GLFW_KEY_F3].pressed)
        demouiToggleShowGraphs();
    
    if(d->keys[GLFW_KEY_F4].pressed)
        demouiToggleGraph();
}

void render()
{
    switch (d->renderPass)
    {
    case PASS_3D:
        demouiStartGPUTimer();
        render2D();
        break;
    case PASS_2D:
        break;
    case PASS_UI:
        {
            demouiBeginRender(d->winSize.x, d->winSize.y, d->winSize.x / d->fbSize.x);
        }
        break;
    case PASS_FLUSH:
        {
            demouiEndRender(d->winSize.x - 200 - 5, 5);
            demouiUpdateGraphs(d->cpuTime, d->frameDelta);
        }
        break;
    default:
        break;
    }
}

int main(void)
{
    DemoParams dmParams = {
        .windowSize = (vec2s){{1280, 960}},
        .title = "Demo 2D",
        .fInit = init,
        .fTerminate = terminate,
        .fUpdate = update,
        .fRender = render,
        .fullscreen = false
    };
    d = demoCreate(dmParams);
    demoRun();
    demoDestroy();

    return 0;
}

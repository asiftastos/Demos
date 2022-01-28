#include "lgl.h"
#include "GLFW/glfw3.h"

static LGLContext* ctx = NULL;

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

    mat4s rotate = glms_rotate_y(GLMS_MAT4_IDENTITY, glm_rad(45.0f));
    mat4s trans = glms_translate_z(GLMS_MAT4_IDENTITY, 20.0f);
    model = glms_mat4_mul(trans, rotate);
    

    mat4s view;
    view = glms_lookat((vec3s){{0.0f, 0.0f, -40.0f}}, GLMS_VEC3_ZERO, GLMS_YUP);
    //model = glms_mat4_mul(view, model);
    
    //glm_ortho(0.0f, d->fbSize[0], 0.0f, d->fbSize[1], 0.1f, 10.0f, proj);
    vec2s size = lglCurrentSize();
    proj = glms_perspective(glm_rad(60.0f), size.x / size.y, 0.1f, 1000.0f);
    proj = glms_mat4_mul(proj, view);

    quadTexture = textureCreate((TextureParams){
        .format = GL_RGB,
        .wrapS = GL_REPEAT,
        .wrapT = GL_REPEAT,
        .minFilter = GL_LINEAR,
        .magFilter = GL_LINEAR,
        .mipmaps = false,
        .path = "wall.jpg"
    });

    textureShader = shaderCreate("demotext.vert", "texture.frag");
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

    printf("Demo texturing terminated\n");
}

void update()
{
    if(lglIsKeyPressed(GLFW_KEY_ESCAPE))
        lglClose();
}

int main(void)
{
    ctx = lglCreateContext((LGLParams) {
            .title="Demo Texture",
            .fullscreen=false,
            .windowSize=(vec2s){{1280.0f, 960.0f}}
    });
    
    init();

    vec4s clearColor = (vec4s){ {0.0f, 0.0f, 0.0f, 1.0f} };
    float depth = 1.0f;

    while (lglIsRunning())
    {
        lglUpdateInput();

        update();

        glClearBufferfv(GL_COLOR, 0, clearColor.raw);
        glClearBufferfv(GL_DEPTH, 0, &depth);

        render2D();

        lglSwapAndPoll();
    }

    terminate();

    lglDestroyContext(ctx);

    return 0;
}

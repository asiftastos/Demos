#define DEMO_COMMON_IMPLEMENTATION
#include "demo.h"
//#include "demoui.h"

typedef void (*PanelAction)();

typedef struct Panel
{
    vec2 position;
    vec2 size;
    float alphas[3];
    vec3 colors[3];
    int color;
    int alpha;
    VertexBuffer* vbo;
    PanelAction clicked;
    double clickedDelay;
}Panel;

static Demo* d = NULL;
//static Demoui* dui = NULL;
static Shader* simple = NULL;
static VertexArray* vao = NULL;
static mat4 model;
static mat4 proj;
static Panel panel;

static void panelCreate()
{
    glm_vec2((vec2){10.0f, 10.0f}, panel.position);
    glm_vec2((vec2){ 100.0f, 50.0f }, panel.size);
    panel.alpha = 0;
    panel.alphas[0] = 0.2f;
    panel.alphas[1] = 0.8f;
    panel.alphas[2] = 0.9f;
    panel.color = 0;
    glm_vec3((vec3){1.0f, 0.0f, 0.0f}, panel.colors[0]);
    glm_vec3((vec3){0.0f, 1.0f, 0.0f}, panel.colors[1]);
    glm_vec3((vec3){0.0f, 0.0f, 1.0f}, panel.colors[2]);
    panel.clickedDelay = 0.0f;
    
    float xmin = 0.0f;
    float xmax = panel.size[0];
    float ymin = 0.0f;
    float ymax = panel.size[1];

    const float vertbuffer[] = 
    {
        xmin, ymin, -1.0f,
        xmax, ymax, -1.0f,
        xmin, ymax, -1.0f,
        xmax, ymax, -1.0f,
        xmin, ymin, -1.0f,
        xmax, ymin, -1.0f
    };

    vao = lglVertexArrayCreate(1, (VertexAttribute[]){
        {
            .index = 0,
            .elementCount = 3,
            .elementType = GL_FLOAT,
            .stride = sizeof(float)*3,
            .offset = 0
        }
    });
    lglVertexArraySet(vao);
    panel.vbo = lglVertexBufferCreate(GL_ARRAY_BUFFER, GL_STATIC_DRAW);
    lglVertexBufferSet(panel.vbo);
    lglVertexBufferData(panel.vbo, sizeof(float), 18, (void*)vertbuffer);
    lglVertexArrayAttributes(vao);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

static bool panelHover()
{
    float xmax = panel.position.x + panel.size.x;
    float ymax = panel.position.y + panel.size.y;
    float mx = d->mouse.position.x;
    float my = d->mouse.position.y;
    return mx > panel.position.x && mx < xmax && my > panel.position.y && my < ymax;
}

static void OnPanelClicked()
{
    printf("Panel was clicked\n");
}

static bool checkboxfps = false;

static void toolUIDraw()
{
    tuiBeginFrame(dui->toolState);

    if(tuiBeginWindow("Stats", (vec4s){{0.0f, 150.0f, 200.0f, 200.0f}}, 0))
    {
        tuiLabel("FPS:");
        tuiLabel("New");
        tuiLabel("TPS:");
        if(tuiButton("Exit"))
            printf("Exit pressed\n");

        if(tuiCheckBox("Show FPS", &checkboxfps))
            printf("Checkbox changed\n");

        tuiEndWindow();
    }

    tuiEndFrame(dui->toolState);
}

void onResize(int width, int height)
{
    proj = glms_ortho(0.0f, width, height, 0.0f, 0.1f, 100.0f);
}

void init()
{
    printf("Demo2d initialized\n");

    d->onResize = onResize;
    
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    simple = shaderCreate("assets/shaders/simple2d.vert", "assets/shaders/simple2d.frag");
    shaderUse(simple);
    shaderSetupUniforms(simple, 4, (const char*[]){"color", "model", "proj", "alpha"});
    glUseProgram(0);

    panelCreate();
    panel.clicked = OnPanelClicked;
    
    model = glms_mat4_identity();
    model = glms_translate_make((vec3s){{panel.position.x, panel.position.y, -1.0f}});
    proj = glms_mat4_identity();
    proj = glms_ortho(0.0f, d->fbSize.x, d->fbSize.y, 0.0f, 0.1f, 100.0f);

    dui = demouiInit(d);
}

void terminate()
{
    demouiTerminate();
    lglVertexBufferDestroy(panel.vbo);
    lglVertexArrayDestroy(vao);
    shaderDestroy(simple);
    printf("Demo2d terminated\n");
}


void update()
{
    if(d->keys[GLFW_KEY_ESCAPE].pressed)
        glfwSetWindowShouldClose(d->window, GLFW_TRUE);
    
    if(d->keys[GLFW_KEY_F3].pressed)
        demouiToggleShowGraphs();
    
    if(d->keys[GLFW_KEY_F4].pressed)
        demouiToggleGraph();
    
    if(panel.clickedDelay > 0.0f){
        panel.clickedDelay -= d->frameDelta;
        if(panel.clickedDelay < 0.0f)
            panel.clickedDelay = 0.0f;
    }

    if(panelHover() && panel.clickedDelay == 0.0f)
    {
        panel.alpha = 1;
        panel.color = 1;
        if(d->mouse.buttons[GLFW_MOUSE_BUTTON_LEFT].pressed)
        {
            panel.alpha = 2;
            panel.color = 2;
            panel.clicked();
            panel.clickedDelay = 1.0f; //1 second 
        }
    }else if(panel.clickedDelay == 0.0f)
    {
        panel.alpha = 0;
        panel.color = 0; 
    }
}

void render()
{
    if(d->renderPass == PASS_3D)
    {
        demouiStartGPUTimer();
    }else if(d->renderPass == PASS_2D)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        shaderUse(simple);
        shaderUploadMatrix(simple, "model", model);
        shaderUploadMatrix(simple, "proj", proj);
        shaderUploadVec3(simple, "color", panel.colors[panel.color]);
        shaderUploadFloat(simple, "alpha", panel.alphas[panel.alpha]);
        lglVertexArraySet(vao);
        lglVertexBufferSet(panel.vbo);
        glDrawArrays(GL_TRIANGLES, 0, panel.vbo->elementCount);
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDisable(GL_BLEND);
        glUseProgram(0);
    }else if(d->renderPass == PASS_UI)
    {
        demouiBeginRender(d->winSize.x, d->winSize.y, d->winSize.x / d->fbSize.x);
        toolUIDraw();
    }else if(d->renderPass == PASS_FLUSH)
    {
        demouiEndRender(d->winSize.x - 200 - 5, 5);
        demouiUpdateGraphs(d->cpuTime, d->frameDelta);
    }
}

int main(void)
{
    d = demoCreate((DemoParams){
        .windowSize = (vec2){1280, 960},
        .title = "Demo 2D",
        .fInit = init,
        .fTerminate = terminate,
        .fUpdate = update,
        .fRender = render,
        .fullscreen = false
    });
    demoRun();
    demoDestroy();

    return 0;
}
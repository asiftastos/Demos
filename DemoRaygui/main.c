#include "demo.h"
#include "demoui.h"
#define RAYGUI_STANDALONE
#include "raygui.h"

static Demo* d = NULL;
static Demoui* dui = NULL;

//============================================================================================
//============================================================================================
//============================================================================================
void init()
{
    printf("Demorayui initialized\n");
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    dui = demouiInit(d);

    GuiSetStyle(DEFAULT, TEXT_SIZE, 18);
}

void terminate()
{
    demouiTerminate();
    printf("Demorayui terminated\n");
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
        break;
    case PASS_2D:
        break;
    case PASS_UI:
        {
            demouiBeginRender(d->winSize.x, d->winSize.y, d->winSize.x / d->fbSize.x);

            Rectangle panelBounds = {1.0f, 1.0f, d->fbSize.x - 1.0f, 40.0f};
            float cellwidth = panelBounds.width / 4.0f;
            GuiPanel(panelBounds);
            if(GuiButton((Rectangle){panelBounds.x + 1.0f + (cellwidth * 0.0f), panelBounds.y + 1.0f, cellwidth - 1.0f, panelBounds.height - 1.0f}, "New"))
                printf("Starting a new world\n");
            if(GuiButton((Rectangle){panelBounds.x + 1.0f + (cellwidth * 1.0f), panelBounds.y + 1.0f, cellwidth - 1.0f, panelBounds.height - 1.0f}, "Load"))
                printf("Loading a world\n");
            if(GuiButton((Rectangle){panelBounds.x + 1.0f + (cellwidth * 2.0f), panelBounds.y + 1.0f, cellwidth - 1.0f, panelBounds.height - 1.0f}, "Options"))
                printf("Change game options\n");
            if(GuiButton((Rectangle){panelBounds.x + 1.0f + (cellwidth * 3.0f), panelBounds.y + 1.0f, cellwidth - 1.0f, panelBounds.height - 1.0f}, "Exit"))
                glfwSetWindowShouldClose(d->window, GLFW_TRUE);
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
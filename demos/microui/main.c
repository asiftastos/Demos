#include "demo.h"
#include "renderer.h"

static Demo* d = NULL;
static mu_Context* muiContext = NULL;
static Renderer* renderer = NULL;
static double frameInterval = 0.0f;
static char frameTime[128] = "dtFrame:";

static void OnResize(int width, int height)
{
    if(renderer != NULL)
    {
        renderer->ortho = glms_ortho(0.0f, (float)width, (float)height, 0.0f, 0.1f, 1.0f);
        renderer->width = width;
        renderer->height = height;
    }
}

void uiDraw()
{
    mu_Command *cmd = NULL;
    while (mu_next_command(muiContext, &cmd))
    {
      switch (cmd->type) 
      {
        case MU_COMMAND_TEXT: r_draw_text(cmd->text.str, cmd->text.pos, cmd->text.color); break;
        case MU_COMMAND_RECT: r_draw_rect(cmd->rect.rect, cmd->rect.color); break;
        case MU_COMMAND_ICON: r_draw_icon(cmd->icon.id, cmd->icon.rect, cmd->icon.color); break;
        case MU_COMMAND_CLIP: r_set_clip_rect(cmd->clip.rect); break;
        default: break;
      }
    }
}

void uiProcess()
{
    if (mu_begin_window_ex(muiContext, "T", mu_rect(0, 100, 150, 200), 0))
    {
        mu_layout_row(muiContext, 2, (int[]) { 50, -1 }, 0);

        mu_label(muiContext, "First:");
        if (mu_button(muiContext, "Button1"))
        {
            printf("Button1 pressed\n");
        }

        mu_label(muiContext, "Second:");
        if (mu_button(muiContext, "Button2"))
        {
            mu_open_popup(muiContext, "My Popup");
        }

        if (mu_begin_popup(muiContext, "My Popup"))
        {
            mu_label(muiContext, "Hello world!");
            mu_end_popup(muiContext);
        }
        mu_layout_row(muiContext, 1, (int[]) { -1 }, -1);
        mu_begin_panel_ex(muiContext, "Stats", 0);
        mu_label(muiContext, frameTime);
        mu_end_panel(muiContext);

        mu_end_window(muiContext);
    }
}

static int text_width(mu_Font f, const char* str, int len)
{
    if(len == -1)
    {
        len = (int)strlen(str);
    }
    return r_get_text_width(str, len);
}

static int text_height(mu_Font f)
{
    return r_get_text_height();
}

void init()
{
    printf("Demo microui initialized\n");
    d->onResize = OnResize;

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    renderer = r_init((int)d->fbSize.x, (int)d->fbSize.y);
    assert(renderer != NULL);

    muiContext = (mu_Context*)malloc(sizeof(mu_Context));
    mu_init(muiContext);
    muiContext->text_width = text_width;
    muiContext->text_height = text_height;
}

void terminate()
{
    r_fini();
    printf("Demo microui terminated\n");
}

void update()
{
    if(d->keys[GLFW_KEY_ESCAPE].pressed)
        glfwSetWindowShouldClose(d->window, GLFW_TRUE);
    
    mu_input_mousemove(muiContext, (int)d->mouse.position.x, (int)d->mouse.position.y);
    mu_input_scroll(muiContext, 0, (int)d->mouse.wheelDelta);

    if(d->mouse.buttons[GLFW_MOUSE_BUTTON_LEFT].down){
        mu_input_mousedown(muiContext, d->mouse.position.x, d->mouse.position.y, MU_MOUSE_LEFT);
    }else{
        mu_input_mouseup(muiContext, d->mouse.position.x, d->mouse.position.y, MU_MOUSE_LEFT);
    }
    
    if(d->mouse.buttons[GLFW_MOUSE_BUTTON_RIGHT].down){
        mu_input_mousedown(muiContext, d->mouse.position.x, d->mouse.position.y, MU_MOUSE_RIGHT);
    }else{
        mu_input_mouseup(muiContext, d->mouse.position.x, d->mouse.position.y, MU_MOUSE_RIGHT);
    }
    
    mu_begin(muiContext);
    uiProcess();
    mu_end(muiContext);
}

void render()
{
    switch (d->renderPass)
    {
    case PASS_3D:
        break;
    case PASS_2D:
        break;
    case PASS_UI:
        {
            uiDraw();
        }
        break;
    case PASS_FLUSH:
        {
            r_present();
            frameInterval += d->frameDelta;
            if(frameInterval >= 1.0f)
            {
                sprintf(frameTime, "dtFrame: %.4fms", d->frameDelta * 1000.0f);
                frameInterval = 0.0f;
            }
        }
        break;
    default:
        break;
    }
}

int main(void)
{
    d = demoCreate((DemoParams){
        .windowSize = (vec2s){{1280, 960}},
        .title = "Demo Microui Rendering",
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

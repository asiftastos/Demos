/*
    TODO
    [x] In drawTextSDF the xpos to advance in the next character does not work with scale
    [x] Each text drawn should have seperate scale matrix depending the size we give in drawTextSDF
        [x] Make a struct text element with neccesary info about the text drawn
        [ ] Letters drawn have big spaces between them
    [ ] rect_pack is not used
*/
#include "demo.h"
#include "vec.h"
#include "stb_rect_pack.h"
#include "stb_truetype.h"

typedef struct VertexTexture
{
    vec3s vertex;
    vec2s texCoord;
}VertexTexture;

typedef vec_t(VertexTexture) vec_vertextexture_t;
typedef vec_t(uint16_t) vec_uint16_t;

typedef struct TextElem
{
    vec2s position;
    float scale;
    char* text;
}TextElem;


static Demo* d = NULL;
static stbtt_packedchar* charinfo = NULL;

#pragma region ====================  SDF  =================================

typedef struct
{
    float advance;
    signed char xoff;
    signed char yoff;
    unsigned int w,h;
    unsigned char *data;
} fontchar;

typedef struct CharBoundingBox
{
    int x0;
    int y0;
    int x1;
    int y1;
}CharBoundingBox;

typedef struct AtlasRect
{
    float x0;
    float y0;
    float x1;
    float y1;
}AtlasRect;


float sdf_size = 64.0;          // the larger this is, the better large font sizes look
static stbtt_fontinfo font;
static float scale;
fontchar fdata[128];
CharBoundingBox charBoundBox[128];
TextElem sdfText;
Shader* sdfShader;
Texture* sdfTexture;
static AtlasRect charInfoRects[128];
static VertexArray* textVao;
static VertexBuffer* textVbo;
static VertexBuffer* textEbo;
static vec_vertextexture_t textBuffer;
static int vertCount;
static vec_uint16_t textIndices;
static int indicesCount;
static mat4s model;
static mat4s proj;

static void loadSDF()
{
    unsigned char* buffer = NULL;
    FILE* f = fopen("assets/fonts/Roboto-Regular.ttf", "rb");
    assert(f != NULL);
    fseek(f, 0, SEEK_END);
    uint64_t len = ftell(f);
    fseek(f, 0, SEEK_SET);
    buffer = calloc(len, sizeof(unsigned char));
    assert(buffer != NULL);
    fread(buffer, 1, len, f);
    fclose(f);

    stbtt_InitFont(&font, buffer, 0);
    scale = stbtt_ScaleForPixelHeight(&font, sdf_size);
    //printf("Font scale: %f\n", scale);

    int texwidth = 0;
    for (int ch=32; ch < 127; ++ch) {
        if(ch == ' ')
            continue;
      fontchar fc;
      int xoff,yoff,w,h, advance;
      fc.data = stbtt_GetCodepointSDF(&font, scale, ch, 6, 180, 180 / 6, &w, &h, &xoff, &yoff);
      fc.xoff = xoff;
      fc.yoff = yoff;
      fc.w = w;
      fc.h = h;
      stbtt_GetCodepointHMetrics(&font, ch, &advance, NULL);
      fc.advance = (float)advance;
      fdata[ch] = fc;
      CharBoundingBox cbb;
      stbtt_GetCodepointBox(&font, ch, &cbb.x0, &cbb.y0, &cbb.x1, &cbb.y1);
      charBoundBox[ch] = cbb;
      texwidth += (int)fc.w;
    }
    free((void*)buffer);

    int texheight = sdf_size;
    int texsize = texwidth*texheight;
    printf("Total texture size: [width]%d [height]%d = [size]%d\n", texwidth, texheight, texsize);
    unsigned char* teximage = (unsigned char*)malloc(texsize * sizeof(unsigned char));
    assert(teximage != NULL);
    memset(teximage, 0, texsize);
    int xpos = 0;
    for(int i = 32; i < 127; ++i)
    {
        fontchar fc = fdata[i];
        for(unsigned int y = 0; y < fc.h; y++)
        {
            for (unsigned int x = 0; x < fc.w; x++)
            {
                unsigned char val = fc.data[(y*fc.w) + x];
                int index = y*texwidth + (xpos + x);
                teximage[index] = val;
            }
        }
        AtlasRect rect;
        rect.x0 = (float)xpos / (float)texwidth;
        rect.y0 = 0.0f;
        rect.x1 = (float)(xpos + fc.w) / (float)texwidth;
        rect.y1 = 1.0f;
        
        charInfoRects[i] = rect;

        xpos += (rect.x1 - rect.x0) + fc.w;
    }
    
    sdfTexture = textureCreate((TextureParams){
        .path = NULL,
        .format = GL_RED,
        .data = teximage,
        .mipmaps = true,
        .wrapS = GL_REPEAT,
        .wrapT = GL_REPEAT,
        .minFilter = GL_LINEAR,
        .magFilter = GL_LINEAR,
        .width = texwidth,
        .height = texheight,
        .pixelStore = 1
    });
}

static void initSDF()
{
    textVao = lglVertexArrayCreate(2, (VertexAttribute[]){
        {.index = 0, .elementCount = 3, .elementType = GL_FLOAT, .stride = sizeof(VertexTexture), .offset = 0},
        {.index = 1, .elementCount = 2, .elementType = GL_FLOAT, .stride = sizeof(VertexTexture), .offset = sizeof(vec3s)}
    });
    textVbo = lglVertexBufferCreate(GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW);
    textEbo = lglVertexBufferCreate(GL_ELEMENT_ARRAY_BUFFER, GL_DYNAMIC_DRAW);

    vec_init(&textBuffer);
    vec_init(&textIndices);
}

static void drawTextSDF(const char* text, vec2s position, size_t size)
{
    float xpos = position.x;
    float ypos = position.y;
    float local_scale = (size / sdf_size) * scale;
    //printf("[scale]%f | [local_scale]%f\n", scale, local_scale);

    int l = strlen(text);
    for(int i = 0; i < l; i++)
    {
        int c = text[i];
        AtlasRect ar = charInfoRects[c];
        fontchar fc = fdata[c];
        CharBoundingBox cbb = charBoundBox[c];

        float xmin = xpos + (local_scale * cbb.x0);
        float xmax = xpos + (local_scale * cbb.x1);
        float ymin = ypos + (local_scale * cbb.y0);
        float ymax = ypos + (local_scale * cbb.y1);

        vec_push(&textBuffer, ((VertexTexture){(vec3s){{xmin, ymin, -1.0f}}, (vec2s){{ar.x0, ar.y1}}}));
        vec_push(&textBuffer, ((VertexTexture){(vec3s){{xmin, ymax, -1.0f}}, (vec2s){{ar.x0, ar.y0}}}));
        vec_push(&textBuffer, ((VertexTexture){(vec3s){{xmax, ymax, -1.0f}}, (vec2s){{ar.x1, ar.y0}}}));
        vec_push(&textBuffer, ((VertexTexture){(vec3s){{xmax, ymin, -1.0f}}, (vec2s){{ar.x1, ar.y1}}}));

        uint16_t lastIndex = textBuffer.length - 4;
        vec_push(&textIndices, lastIndex);
        vec_push(&textIndices, lastIndex + 2);
        vec_push(&textIndices, lastIndex + 1);
        vec_push(&textIndices, lastIndex);
        vec_push(&textIndices, lastIndex + 3);
        vec_push(&textIndices, lastIndex + 2);

        xpos += local_scale * fc.advance;
    }

    vertCount = textBuffer.length;
    indicesCount = textIndices.length;
}

static void drawSDF()
{
    lglVertexArraySet(textVao);
    lglVertexBufferSet(textVbo);
    lglVertexBufferData(textVbo, sizeof(VertexTexture), vertCount, textBuffer.data);
    lglVertexArrayAttributes(textVao);
    lglVertexBufferSet(textEbo);
    lglVertexBufferData(textEbo, sizeof(uint16_t), indicesCount, textIndices.data);

    shaderUse(sdfShader);
    shaderUploadMatrix(sdfShader, "model", model);
    shaderUploadMatrix(sdfShader, "proj", proj);
    textureSet(sdfTexture);
    glDrawElements(GL_TRIANGLES, indicesCount, GL_UNSIGNED_SHORT, NULL);
    glBindVertexArray(0);

    vec_clear(&textBuffer);
    vec_clear(&textIndices);
}

#pragma endregion

//===============================================================================
//              DEMO
// ==============================================================================
void demoInit()
{
    printf("Demotext initialized\n");
    glClearColor(0.4f, 0.4f, 0.4f, 1.0f);

    //glm_translate(model, (vec3){10.0f, 10.0f, 0.0f});
    proj = glms_ortho(0.0f, d->fbSize.x, 0.0f, d->fbSize.y, 0.1f, 100.0f);

    loadSDF();
    initSDF();
    sdfShader = shaderCreate("assets/shaders/demotext.vert", "assets/shaders/demosdf.frag");
    shaderUse(sdfShader);
    shaderSetupUniforms(sdfShader, 2, (const char*[]){"model", "proj"});
    
    model = GLMS_MAT4_IDENTITY;//glms_scale(GLMS_MAT4_IDENTITY, (vec3s){{0.8f, 0.8f, 1.0f}});  // text scale matrix in general for all
    
    sdfText.text = "Konstantinos";
    sdfText.position = (vec2s){{0.0f, 0.0f}};
    sdfText.scale = 44.0f;
}

void demoTerminate()
{
    vec_deinit(&textBuffer);
    vec_deinit(&textIndices);

    textureDestroy(sdfTexture);
    lglVertexBufferDestroy(textEbo);
    lglVertexBufferDestroy(textVbo);
    lglVertexArrayDestroy(textVao);
    if(charinfo)
        free((void*)charinfo);
    printf("Demotext terminated\n");
}

void demoUpdate()
{
    if(d->keys[GLFW_KEY_ESCAPE].pressed)
        glfwSetWindowShouldClose(d->window, GLFW_TRUE);
}

void demoRender()
{
    //needed otherwise draws all the rectangle around the letter.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    drawTextSDF(sdfText.text, sdfText.position, sdfText.scale);

    drawSDF();
}

int main(void)
{
    d = demoCreate((DemoParams){
        .windowSize = (vec2s){{1280, 960}},
        .title = "Demo Text Rendering",
        .fInit = demoInit,
        .fTerminate = demoTerminate,
        .fUpdate = demoUpdate,
        .fRender = demoRender,
        .fullscreen = false
    });
    demoRun();
    demoDestroy();
    return 0;
}
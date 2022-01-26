#include "demo.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "renderer.h"
#include "atlas.inl"

typedef struct VertexColorTexture
{
    vec3s position;
    vec3s color;
    vec2s texcoords;
}VertexColorTexture;

typedef vec_t(VertexColorTexture) vec_vct_t;
typedef vec_t(uint32_t) vec_uint32_t;

static vec_vct_t vBuffer;
static vec_uint32_t iBuffer;



static Texture* atlasImage = NULL;
static Shader* shader = NULL;
static VertexArray* vao = NULL;
static VertexBuffer* vbo = NULL;
static VertexBuffer* ebo = NULL;

static Renderer renderer;

Renderer* r_init(int width, int height) 
{
  renderer.width = width;
  renderer.height = height;
  renderer.model = glms_mat4_identity();
  renderer.ortho = glms_ortho(0.0f, (float)width, (float)height, 0.0f, 0.1f, 1.0f);

    uint32_t rgba8_size = ATLAS_WIDTH * ATLAS_HEIGHT * 4;
    uint32_t* rgba8_pixels = (uint32_t*) malloc(rgba8_size);
    for (int y = 0; y < ATLAS_HEIGHT; y++) {
        for (int x = 0; x < ATLAS_WIDTH; x++) {
            int index = y*ATLAS_WIDTH + x;
            rgba8_pixels[index] = 0x00FFFFFF | ((uint32_t)atlas_texture[index]<<24);
        }
    }

    //stbi_write_jpg("microuiatlas.jpg", ATLAS_WIDTH, ATLAS_HEIGHT, 1, atlas_texture, 85);
    //stbi_write_png("microuiatlas.png", ATLAS_WIDTH, ATLAS_HEIGHT, 1, atlas_texture, 0);

  /* init texture */
  atlasImage = textureCreate((TextureParams){
      .path=NULL,
      .width=ATLAS_WIDTH,
      .height=ATLAS_HEIGHT,
      .format=GL_RGBA,
      .pixelStore=0,
      .minFilter=GL_NEAREST,
      .magFilter=GL_NEAREST,
      .wrapS=GL_REPEAT,
      .wrapT=GL_REPEAT,
      .data=(unsigned char*)rgba8_pixels,
      .freeData=true,
      .mipmaps=false
  });

  shader = shaderCreate("assets/shaders/microui.vert", "assets/shaders/microui.frag");
  shaderUse(shader);
  shaderSetupUniforms(shader, 2, (const char*[]){"model", "proj"});

  vao = lglVertexArrayCreate(3, (VertexAttribute[]){
      {.index=0, .elementCount=3, .elementType=GL_FLOAT, .stride=sizeof(VertexColorTexture), .offset=0},
      {.index=1, .elementCount=3, .elementType=GL_FLOAT, .stride=sizeof(VertexColorTexture), .offset=sizeof(vec3s)},
      {.index=2, .elementCount=2, .elementType=GL_FLOAT, .stride=sizeof(VertexColorTexture), .offset=sizeof(vec3s)*2},
  });

  vbo = lglVertexBufferCreate(GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW);
  ebo = lglVertexBufferCreate(GL_ELEMENT_ARRAY_BUFFER, GL_DYNAMIC_DRAW);

  vec_init(&vBuffer);
  vec_init(&iBuffer);

  return &renderer;
}

void r_fini()
{
    vec_deinit(&vBuffer);
    vec_deinit(&iBuffer);
    textureDestroy(atlasImage);
    lglVertexBufferDestroy(vbo);
    lglVertexBufferDestroy(ebo);
    lglVertexArrayDestroy(vao);
    shaderDestroy(shader);
}

static void flush(void) {
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_SCISSOR_TEST);
  glEnable(GL_TEXTURE_2D);

  lglVertexArraySet(vao);
  lglVertexBufferSet(vbo);
  lglVertexBufferData(vbo, sizeof(VertexColorTexture), vBuffer.length, vBuffer.data);
  lglVertexArrayAttributes(vao);
  lglVertexBufferSet(ebo);
  lglVertexBufferData(ebo, sizeof(uint32_t), iBuffer.length, iBuffer.data);

  shaderUse(shader);
  shaderUploadMatrix(shader, "model", renderer.model);
  shaderUploadMatrix(shader, "proj", renderer.ortho);
  textureSet(atlasImage);
  glDrawElements(GL_TRIANGLES, iBuffer.length, GL_UNSIGNED_INT, NULL);

  vec_clear(&vBuffer);
  vec_clear(&iBuffer);
}


static void push_quad(mu_Rect dst, mu_Rect src, mu_Color color) {
    float z = -1.0f;    
  /* update texture buffer */
    float x = src.x / (float) ATLAS_WIDTH;
    float y = src.y / (float) ATLAS_HEIGHT;
    float w = src.w / (float) ATLAS_WIDTH;
    float h = src.h / (float) ATLAS_HEIGHT;
      
    vec_push(&vBuffer, ((VertexColorTexture){
        .position = (vec3s){{(float)dst.x, (float)dst.y, z}},
        .color = (vec3s){{(float)color.r / 255.0f, (float)color.g / 255.0f, (float)color.b / 255.0f}},
        .texcoords = (vec2s){{x, y}}
    }));
    vec_push(&vBuffer, ((VertexColorTexture){
        .position = (vec3s){{(float)(dst.x + dst.w), (float)dst.y, z}},
        .color = (vec3s){{(float)color.r / 255.0f, (float)color.g / 255.0f, (float)color.b / 255.0f}},
        .texcoords = (vec2s){{x + w, y}}
    }));
    vec_push(&vBuffer, ((VertexColorTexture){
        .position = (vec3s){{(float)(dst.x + dst.w), (float)(dst.y + dst.h), z}},
        .color = (vec3s){{(float)color.r / 255.0f, (float)color.g / 255.0f, (float)color.b / 255.0f}},
        .texcoords = (vec2s){{x + w, y + h}}
    }));
    vec_push(&vBuffer, ((VertexColorTexture){
        .position = (vec3s){{(float)dst.x, (float)(dst.y + dst.h), z}},
        .color = (vec3s){{(float)color.r / 255.0f, (float)color.g / 255.0f, (float)color.b / 255.0f}},
        .texcoords = (vec2s){{x, y + h}}
    }));

    uint32_t index = vBuffer.length - 4;
    vec_push(&iBuffer, index);
    vec_push(&iBuffer, index + 1);
    vec_push(&iBuffer, index + 2);
    vec_push(&iBuffer, index);
    vec_push(&iBuffer, index + 2);
    vec_push(&iBuffer, index + 3);
}


void r_draw_rect(mu_Rect rect, mu_Color color) {
  push_quad(rect, atlas[ATLAS_WHITE], color);
}


void r_draw_text(const char *text, mu_Vec2 pos, mu_Color color) {
  mu_Rect dst = { pos.x, pos.y, 0, 0 };
  for (const char *p = text; *p; p++) {
    if ((*p & 0xc0) == 0x80) { continue; }
    int chr = mu_min((unsigned char) *p, 127);
    mu_Rect src = atlas[ATLAS_FONT + chr];
    dst.w = src.w;
    dst.h = src.h;
    push_quad(dst, src, color);
    dst.x += dst.w;
  }
}


void r_draw_icon(int id, mu_Rect rect, mu_Color color) {
  mu_Rect src = atlas[id];
  int x = rect.x + (rect.w - src.w) / 2;
  int y = rect.y + (rect.h - src.h) / 2;
  push_quad(mu_rect(x, y, src.w, src.h), src, color);
}


int r_get_text_width(const char *text, int len) {
  int res = 0;
  for (const char *p = text; *p && len--; p++) {
    if ((*p & 0xc0) == 0x80) { continue; }
    int chr = mu_min((unsigned char) *p, 127);
    res += atlas[ATLAS_FONT + chr].w;
  }
  return res;
}


int r_get_text_height(void) {
  return 22;
}


void r_set_clip_rect(mu_Rect rect) {
  //flush();
  glScissor(rect.x, renderer.height - (rect.y + rect.h), rect.w, rect.h);
}

void r_present(void) {
  flush();
}

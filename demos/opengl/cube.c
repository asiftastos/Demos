/*
    World is on positive side of all axis (X+, Y+, Z+)
    Block's position is always the block's bottom-left-back corner (minimum coords)
    
    TODO
    [x] add faces depending of the position of cube
    [x] add color to the block struct depending the side
    [x] vertices array should be allocated depending the free sides of the block
    [x] add indices to addface function
    [x] setting a bitfield for a neighbour side effets the next side too and both are not renderer
    [ ] get block properties from files,like side color (json,toml)
*/
#include "stdint.h"
#include "demo.h"
#include "demoui.h"

// N = the number of bit to change [1..8]
#define SET_BIT(BF, N) BF |= ((char)0x01 << N)
#define CLR_BIT(BF, N) BF &= ~((char)0x01 << N)
#define IS_BIT_SET(BF, N) ((BF >> N) & 0x1)
#define TOGGLE_BIT(BF, N) BF ^= ((char)0x01 << N)

#define BLOCK_SIZE 4

enum Faces {
    SOUTH,
    EAST,
    NORTH,
    WEST,
    TOP,
    BOTTOM
};

typedef vec3s (*FColor)(enum Faces face);

typedef struct Block {
    VertexArray* vao;
    VertexBuffer* vbo;
    VertexBuffer* ebo;
    mat4s model;
    vec3s position;     // position in a map's grid (map's local), not vertex coordinates 
    char neigbours;     // bitfield setting for neigbour existance, first 6 bits are for the 6 sides of the cube
    FColor color;
    int vertCount;
    int indicesCount;
}Block;

static vec3s FColorStone(enum Faces face)
{
    if(face == TOP)
        return (vec3s){{0.0f, 1.0f, 0.0f}};
    return (vec3s){{0.6f, 0.4f, 0.0f}};
}

static Demo* d = NULL;
static Demoui* dui = NULL;

static Block *stone = NULL;

static Shader* cubeShader;
static mat4s proj;

static VertexArray* debugVao;
static VertexBuffer* debugVbo;
static mat4s debugModel;

static vec3s CUBE_VERTICES[] = {
    {{0.0f, 0.0f, 1.0f}},   //z+
    {{1.0f, 0.0f, 1.0f}},
    {{1.0f, 1.0f, 1.0f}},
    {{0.0f, 1.0f, 1.0f}},

    {{0.0f, 0.0f, 0.0f}},   //z-
    {{1.0f, 0.0f, 0.0f}},
    {{1.0f, 1.0f, 0.0f}},
    {{0.0f, 1.0f, 0.0f}}
};

static int CUBE_INDICES[] = {
    0, 1, 2, 3,   //z+ (SOUTH)
    1, 5, 6, 2,   //x+ (EAST)
    4, 7, 6, 5,   //z- (NORTH)
    4, 0, 3, 7,   //x- (WEST)
    3, 2, 6, 7,   //y+ (TOP)
    4, 5, 1, 0    //y- (BOTTOM)
};

static void AddFace(vec3s* verts, uint32_t* indices, Block* block, int face)
{
    int faceIndex = face * 4;
    vec3s worldPosMin = glms_vec3_muladds(block->position, (float)BLOCK_SIZE, GLMS_VEC3_ZERO);
    vec3s worldPosMax = glms_vec3_adds(worldPosMin, (float)BLOCK_SIZE);

    vec3s c = block->color(face);
    int vindex = block->vertCount / 2;
    verts[block->vertCount++] = glms_vec3_mul(CUBE_VERTICES[CUBE_INDICES[faceIndex]], worldPosMax);
    verts[block->vertCount++] = c;
    verts[block->vertCount++] = glms_vec3_mul(CUBE_VERTICES[CUBE_INDICES[faceIndex + 1]], worldPosMax);
    verts[block->vertCount++] = c;
    verts[block->vertCount++] = glms_vec3_mul(CUBE_VERTICES[CUBE_INDICES[faceIndex + 2]], worldPosMax);
    verts[block->vertCount++] = c;
    verts[block->vertCount++] = glms_vec3_mul(CUBE_VERTICES[CUBE_INDICES[faceIndex + 3]], worldPosMax);
    verts[block->vertCount++] = c;
    
    indices[block->indicesCount++] = vindex;
    indices[block->indicesCount++] = vindex + 1;
    indices[block->indicesCount++] = vindex + 2;
    indices[block->indicesCount++] = vindex;
    indices[block->indicesCount++] = vindex + 2;
    indices[block->indicesCount++] = vindex + 3;
}

static void render3D()
{
    //stone->model = glms_mat4_mul(stone->model, glms_translate(GLMS_MAT4_IDENTITY, (vec3s){{center.x, center.y, center.z}}));
    //stone->model = glms_rotate_y(stone->model, glm_rad(-16.5f * (float)d->frameDelta));
    //stone->model = glms_mat4_mul(stone->model, glms_translate(GLMS_MAT4_IDENTITY, (vec3s){{-center.x, -center.y, -center.z}}));

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    shaderUse(cubeShader);
    shaderUploadMatrix(cubeShader, "model", stone->model);
    shaderUploadMatrix(cubeShader, "proj", proj);
    lglVertexArraySet(stone->vao);
    lglVertexBufferSet(stone->ebo);
    glDrawElements(GL_TRIANGLES, stone->indicesCount, GL_UNSIGNED_INT, NULL);

    //debug
    glEnable(GL_LINE_SMOOTH);
    glLineWidth(2.0f);
    shaderUploadMatrix(cubeShader, "model", debugModel);
    lglVertexArraySet(debugVao);
    glDrawArrays(GL_LINES, 0, debugVbo->elementCount);
    glBindVertexArray(0);
    glDisable(GL_LINE_SMOOTH);
    glLineWidth(1.0f);
    
    glUseProgram(0);
    glDisable(GL_CULL_FACE);
}


static void blockDataBuffers(Block* block, vec3s* verts, uint32_t* indices, int stribe)
{
    printf("Uploading %d vertices\n", block->vertCount);
    block->vao = lglVertexArrayCreate(2, (VertexAttribute[]){
        {.index=0, .elementCount=3, .elementType=GL_FLOAT, .stride=stribe, .offset=0},
        {.index=1, .elementCount=3, .elementType=GL_FLOAT, .stride=stribe, .offset=sizeof(vec3s)}
    });
    lglVertexArraySet(block->vao);
    block->vbo = lglVertexBufferCreate(GL_ARRAY_BUFFER, GL_STATIC_DRAW);
    lglVertexBufferSet(block->vbo);
    lglVertexBufferData(block->vbo, sizeof(vec3s), block->vertCount, verts);
    lglVertexArrayAttributes(block->vao);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //indices
    printf("Uploading %d indices\n", block->indicesCount);
    block->ebo = lglVertexBufferCreate(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW);
    lglVertexBufferSet(block->ebo);
    lglVertexBufferData(block->ebo, sizeof(uint32_t), block->indicesCount, indices);
}

void init()
{
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    dui = demouiInit(d);

    stone = (Block*)calloc(1, sizeof(Block));
    assert(stone != NULL);

    stone->position = (vec3s){{0.0f, 0.0f, 0.0f}};
    stone->model = GLMS_MAT4_IDENTITY;
    stone->neigbours = 0;
    stone->color = FColorStone;
    
    mat4s view;
    view = glms_lookat((vec3s){{24.0f, 36.0f, 70.0f}}, GLMS_VEC3_ZERO, GLMS_YUP);
    proj = glms_perspective(glm_rad(45.0f), d->fbSize.x / d->fbSize.y, 0.1f, 1000.0f);
    proj = glms_mat4_mul(proj, view);

    cubeShader = shaderCreate("assets/shaders/cube.vert", "assets/shaders/cube.frag");
    shaderUse(cubeShader);
    shaderSetupUniforms(cubeShader, 2, (const char*[]){"model", "proj"});
    glUseProgram(0);
    
    //SET_BIT(stone->neigbours, (SOUTH + 1));
    //SET_BIT(stone->neigbours, (EAST + 1));
    //SET_BIT(stone->neigbours, (NORTH + 1));
    //SET_BIT(stone->neigbours, (WEST + 1));
    //SET_BIT(stone->neigbours, (TOP + 1));
    //SET_BIT(stone->neigbours, (BOTTOM + 1));

    int emptyFaces = 0;
    printf("[");
    for(int i = 1; i < 7; i++)
    {
        if(IS_BIT_SET(stone->neigbours, i))
        {
            printf("+");
            continue;
        }
        printf(".");
        emptyFaces++;
    }
    printf("]\t");
    printf("Empty sides: %d\n", emptyFaces);
    
    uint32_t* indices = calloc(1, emptyFaces * 6 * sizeof(uint32_t));
    assert(indices != NULL);
    vec3s* quad = calloc(1, emptyFaces * 2 * 4 * sizeof(vec3s));
    assert(quad != NULL);

    stone->indicesCount = 0;
    stone->vertCount = 0;

    //int index = 0;
    for(int d = 1; d < 7; d++)
    {
        if(IS_BIT_SET(stone->neigbours, (d)))
            continue;

        AddFace(quad, indices, stone, d - 1);
    }
        
    printf("Vert count: %d\n", stone->vertCount);
    printf("Indices count: %d\n", stone->indicesCount);

    blockDataBuffers(stone, quad, indices, sizeof(vec3s) * 2);

    free(indices);
    free(quad);

    // debug
    debugModel = GLMS_MAT4_IDENTITY;

    float dbgverts[] = {
        0.0f, 0.0f, 0.0f,  1.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 8.0f,  1.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 0.0f,  1.0f, 1.0f, 1.0f,
        8.0f, 0.0f, 0.0f,  1.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 0.0f,  1.0f, 1.0f, 1.0f,
        0.0f, 8.0f, 0.0f,  1.0f, 1.0f, 1.0f
    };

    debugVao = lglVertexArrayCreate(2, (VertexAttribute[]){
        {.index=0, .elementCount=3, .elementType=GL_FLOAT, .stride=sizeof(float)*6, .offset=0},
        {.index=1, .elementCount=3, .elementType=GL_FLOAT, .stride=sizeof(float)*6, .offset=sizeof(float)*3},
    });
    lglVertexArraySet(debugVao);
    debugVbo = lglVertexBufferCreate(GL_ARRAY_BUFFER, GL_STATIC_DRAW);
    lglVertexBufferSet(debugVbo);
    lglVertexBufferData(debugVbo, sizeof(float), 36, dbgverts);
    lglVertexArrayAttributes(debugVao);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);


    printf("Demo cube initialized\n");
}

void terminate()
{
    lglVertexBufferDestroy(debugVbo);
    lglVertexArrayDestroy(debugVao);
    lglVertexBufferDestroy(stone->ebo);
    lglVertexBufferDestroy(stone->vbo);
    lglVertexArrayDestroy(stone->vao);
    shaderDestroy(cubeShader);

    if(stone)
        free(stone);

    demouiTerminate();
    printf("Demo cube terminated\n");
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
        render3D();
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
    d = demoCreate((DemoParams){
        .windowSize = (vec2s){{1280, 960}},
        .title = "Demo Text Rendering",
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

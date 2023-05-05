#pragma once

typedef struct Text
{
    char text[32];
}Text;

typedef struct Position{
    double x, y;
}Position;

ECS_COMPONENT_EXTERN(Text);
ECS_COMPONENT_EXTERN(Position);

void componentsInit(ecs_world_t* world);

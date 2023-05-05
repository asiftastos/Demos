#include "flecs.h"
#include "components.h"
#include "systems.h"

ECS_COMPONENT_DECLARE(Text);
ECS_COMPONENT_DECLARE(Position);

void PrintText(ecs_iter_t* it)
{
    Text* t = ecs_term(it, Text, 1);

    for(int i = 0; i < it->count; i++)
    {
        printf("%s\n", t->text);
    }
}

void Move(ecs_iter_t *it)
{
    Position *p = ecs_term(it, Position, 1);

    for(int i = 0; i < it->count; i++)
    {
        const char* name = ecs_get_name(it->world, it->entities[i]);
        if(stricmp("Player", name) == 0)
        {
            p[i].x++;
        }else{
            p[i].x -= 4.0;
        }
        printf("Position of [%s]: %lf, %lf\n", name, p[i].x, p[i].y);
        Text tt;
        sprintf(tt.text, "%s is moving", name);
        ecs_set_id(it->world, it->entities[i], ecs_id(Text), sizeof(Text), &tt);  // performs memcpy, component will be added if not exists
    }
}

void componentsInit(ecs_world_t* world)
{
    ECS_COMPONENT_DEFINE(world, Text);
    ECS_COMPONENT_DEFINE(world, Position);
}

void systemsInit(ecs_world_t* world)
{
    ecs_entity_t move_sys = ecs_system_init(world, &(ecs_system_desc_t){
        .query.filter.terms = {{ecs_id(Position)}},
        .callback = Move
    });
    ecs_add_id(world, move_sys, EcsOnUpdate);

    ecs_entity_t print_sys = ecs_system_init(world, &(ecs_system_desc_t){
        .query.filter.terms = {{ecs_id(Text)}},
        .callback = PrintText
    });
    ecs_add_id(world, print_sys, EcsPreUpdate);
}

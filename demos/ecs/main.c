#include <stdio.h>
#include <string.h>
#include "flecs.h"
#include "components.h"
#include "systems.h"

int main(int argc, const char **argv)
{
    ecs_world_t *world = ecs_init();

    componentsInit(world);
    systemsInit(world);

    ECS_ENTITY(world, Player, Text);
    
    ecs_set(world, Player, Text, {"Player"});

    ecs_set_target_fps(world, 8);   // set how many ecs_progress will be ticked in a second, only works for ecs_progress

    bool running = true;
    //ecs_progress runs the ecs_pipeline (all systems that are registered) for one tick at a time
    //alternate is to run each system manualy with ecs_run
    while(running)
    {
        running = ecs_progress(world, 1.0f);
        ecs_world_info_t* info = (ecs_world_info_t*)ecs_get_world_info(world);
        printf("world frame count total: %d\n", info->frame_count_total);
    }

    ecs_fini(world);

    return 0;
}
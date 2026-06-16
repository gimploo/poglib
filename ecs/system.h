#pragma once
#include "./common.h"

void ecs_add_system(ecs_t * const self, const ecs_system_t system)
{
    ASSERT(self);
    ASSERT(system.callback);

    ecs_systemmanager_t *const x = &self->managers.systemmanager;
    ASSERT(x->count < ECS_SYSTEM_MAX_COUNT);

    x->systems[x->count++] = system;
}


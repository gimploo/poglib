#pragma once
#include "./common.h"


void ecs_add_system(ecs_t * const self, const ecs_system_callback callback)
{
    ASSERT(self);
    ASSERT(callback);

    ecs_systemmanager_t *const x = &self->managers.systemmanager;
    ASSERT(x->count < ECS_SYSTEM_MAX_COUNT);

    x->systems[x->count++].callback = callback;
}

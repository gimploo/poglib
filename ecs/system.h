#pragma once
#include "poglib/basic/ds/slot.h"
#include "poglib/ecs/component.h"

typedef void (*ecs_system_callback)(slot_t * const componentpool);

typedef struct ecs_systemmanager_t ecs_systemmanager_t;
struct ecs_systemmanager_t {
    slot_t systems;
};

typedef struct ecs_system_t ecs_system_t;
struct ecs_system_t {
    ecs_component_type type;
    ecs_system_callback callback;
};


ecs_systemmanager_t     ecs_systemmanager(arena_t * const arena);
void                    ecs_systemmanager_add(ecs_systemmanager_t *const self, const ecs_system_t system);
void                    ecs_systemmanager_update(ecs_systemmanager_t *const self, const ecs_system_t system);
void                    ecs_systemmanager_delete(ecs_systemmanager_t *const self, const ecs_system_t system);


#ifndef IGNORE_ECS_SYSTEM_IMPLEMENTATION

ecs_systemmanager_t ecs_systemmanager(arena_t * const arena)
{
    ASSERT(arena);
    ecs_systemmanager_t o = {
        .systems = slot_init(ECS_CMP_COUNT, sizeof(ecs_system_callback), arena)
    };
    return o;
}

void ecs_systemmanager_add(ecs_systemmanager_t *const self, const ecs_system_t system)
{
    slot_insert(
        &self->systems, 
        1 << system.type, 
        system.callback,
        sizeof(ecs_system_t));
}

void ecs_systemmanager_update(ecs_systemmanager_t *const self, const ecs_system_t system)
{
    slot_update(
        &self->systems, 
        1 << system.type, 
        system.callback,
        sizeof(ecs_system_t));
}

void ecs_systemmanager_delete(ecs_systemmanager_t *const self, const ecs_system_t system)
{
    slot_delete(&self->systems, 1 << system.type);
}

#endif

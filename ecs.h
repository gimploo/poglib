#pragma once
#include <poglib/basic.h>
#include "./ecs/entity.h"
#include "./ecs/component.h"
#include "poglib/ecs/common.h"
#include "poglib/ecs/component/types.h"

ecs_t        ecs_init(void);
void         ecs_update(ecs_t *const self);
u32             ecs_entity_add(ecs_t * const self, const ecs_componentbundle_t component_config);
void            ecs_entity_remove(ecs_t * const self, const u32 entityId);
void         ecs_destroy(ecs_t * const self);

#ifndef IGNORE_ECS_IMPLEMENTATION

ecs_t ecs_init(void)
{
    //FIXME: WTF it needs 500 MB ??
    //TODO: Figure out a way to visualize know where memory is distributed in the system 
    arena_t arena = arena_init(NULL, 500 * MB);
    ecs_t result = {
        .internal = {
            .entity_generator_counter = ECS_ENTITY_INVALID_ID,
        },
        .managers = {
            .entitymanager = ecs_entitymanager(&arena),
            .componentmanager = ecs_componentmanager(&arena),
            .systemmanager = {0},
        }
    };
    result.arena = arena;
    return result;
}

u32 ecs_entity_add(ecs_t * const self, const ecs_componentbundle_t component_config)
{
    const ecs_entity_t new_entity = {
        .id = ++self->internal.entity_generator_counter,
        .component_signature = component_config.signature,
    };
    ecs_entitymanager_add(
        &self->managers.entitymanager, 
        new_entity
    );

    ecs_componentmanager_add(
        &self->managers.componentmanager,
        new_entity.id,
        component_config
    );
    return new_entity.id;
}

void ecs_entity_remove(ecs_t * const self, const u32 entityId)
{
    ecs_entitymanager_remove(&self->managers.entitymanager, entityId);
    ecs_componentmanager_removeall(
        &self->managers.componentmanager,
        entityId
    );
}

void ecs_update(ecs_t *const self)
{
    ASSERT(self);

    const ecs_systemmanager_t * const manager = &self->managers.systemmanager;
    for (u8 idx = 0; idx < ECS_SYSTEM_MAX_COUNT; idx++)
    {
        if (!manager->count || !manager->systems[idx].callback)
            continue;

        manager->systems[idx].callback(
            &self->managers.componentmanager
        );
    }
}


void ecs_destroy(ecs_t * const self)
{
    arena_destroy(&self->arena);
}

#endif

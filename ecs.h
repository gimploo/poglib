#pragma once
#include <poglib/basic.h>
#include "./ecs/entity.h"
#include "./ecs/component.h"
#include "./ecs/system.h"
#include "poglib/ecs/component/types.h"

ecs_t        ecs_init(void);
void         ecs_update(ecs_t *const self);
void         ecs_entity_add(ecs_t * const self, const ecs_componentbundle_t component_config);
void         ecs_entity_remove(ecs_t * const self, const u32 entityId);
void         ecs_destroy(ecs_t * const self);

#ifndef IGNORE_ECS_IMPLEMENTATION

ecs_t ecs_init(void)
{
    arena_t arena = arena_init(NULL, 1 * GB);
    ecs_t result = {
        .internal = {
            .entity_generator_counter = 0,
        },
        .managers = {
            .entitymanager = ecs_entitymanager(&arena),
            .componentmanager = ecs_componentmanager(&arena),
            .systemmanager = ecs_systemmanager(&arena),
        }
    };
    result.arena = arena;
    return result;
}

void ecs_entity_add(ecs_t * const self, const ecs_componentbundle_t component_config)
{
    const ecs_entity_t new_entity = {
        .id = self->internal.entity_generator_counter++,
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
    slot_iterator(&self->managers.componentmanager.componentpool_slots, component_pool)
    {
        if(slot_is_index_occupied(&self->managers.systemmanager.systems, slot_iterator_index)) {
            ecs_system_callback callback = slot_get_value(&self->managers.systemmanager.systems, slot_iterator_index);
            callback(component_pool, &self->managers.componentmanager);
        }
    }
}

void ecs_destroy(ecs_t * const self)
{
    arena_destroy(&self->arena);
}

#endif

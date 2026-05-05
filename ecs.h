#pragma once
#include <poglib/basic.h>

#include "./ecs/entity.h"
#include "./ecs/component.h"
#include "./ecs/system.h"
#include "poglib/basic/ds/slot.h"

typedef struct {
    arena_t arena;
    struct {
        ecs_entitymanager_t         entitymanager;
        ecs_componentmanager_t      componentmanager;
        ecs_systemmanager_t         systemmanager;
    } managers;

    struct {
        u32 entity_generator_counter;
    } internal;
} ecs_t;


ecs_t           ecs_init(void);
void            ecs_update(ecs_t *const self);
void            ecs_entity_add(ecs_t * const self, const u32 component_signature);
void            ecs_entity_remove(ecs_t * const self, const u32 entityId);
void            ecs_destroy(ecs_t * const self);


#ifndef IGNORE_ECS_IMPLEMENTATION

ecs_t ecs_init(void)
{
    arena_t arena = arena_init(NULL, 1 * KB);

    return (ecs_t) {
        .arena = arena,
        .internal = {
            .entity_generator_counter = -1,
        },
        .managers = {
            .entitymanager = ecs_entitymanager(&arena),
            .componentmanager = ecs_componentmanager(&arena),
            .systemmanager = ecs_systemmanager(&arena),
        }
    };
}

void ecs_entity_add(ecs_t * const self, const u32 component_signature)
{
    const ecs_entity_t new_entity = {
        .id = ++self->internal.entity_generator_counter,
        .component_signature = component_signature,
    };
    ecs_entitymanager_add(
        &self->managers.entitymanager, 
        new_entity
    );

    ecs_componentmanager_add(
        &self->managers.componentmanager,
        new_entity.id,
        component_signature
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
            callback(component_pool);
        }
    }
}


void ecs_destroy(ecs_t * const self)
{
    arena_destroy(&self->arena);
}

#endif

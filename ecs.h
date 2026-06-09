#pragma once
#include <poglib/basic.h>
#include "./ecs/entity.h"
#include "./ecs/component.h"
#include "poglib/ecs/common.h"
#include "poglib/ecs/component/types.h"
#include "poglib/input/commandqueue.h"

ecs_t           ecs_init(void);

u32                 ecs_entity_add(ecs_t * const self, const ecs_componentbundle_t component_config);
void                ecs_entity_remove(ecs_t * const self, const u32 entityId);

ecs_entity_query_t   ecs_entity_query_components(ecs_t *const self, const u32 entity_id, const u32 component_signature);

void                ecs_set_active_camera(ecs_t *const self, const u32 entity_id);
glcamera_t *        ecs_get_active_camera(ecs_t *const self);

void                ecs_set_active_commandqueue(ecs_t *const self, commandqueue_t *const commandqueue);

void                ecs_patch_entity(ecs_t *const self, const u32 entity_id, const ecs_cmp_patch_payload_t request);

void            ecs_update(ecs_t *const self);
void            ecs_destroy(ecs_t * const self);

#ifndef IGNORE_ECS_IMPLEMENTATION

ecs_t ecs_init(void)
{
    //FIXME: WTF it needs 500 MB ??
    //TODO: Figure out a way to visualize know where memory is distributed in the system 
    arena_t arena = arena_init(NULL, 500 * MB);
    ecs_t result = {
        .internal = {
            .entity_generator_counter = ECS_ENTITY_INVALID_ID,
            .active_camera = NULL,
            .active_commandqueue = NULL,
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


glcamera_t * ecs_get_active_camera(ecs_t *const self)
{
    ASSERT(self->internal.active_camera);
    return self->internal.active_camera;
}

void ecs_set_active_camera(ecs_t *const self, const u32 entity_id)
{
    const ecs_entity_query_t view = ecs_entity_query_components(self, entity_id, ECS_CMP_CAMERA);
    ecs_component_camera_t *const camera = view.entity_cmp_data[ECS_CMP_CAMERA_IDX];
    ASSERT(camera);

    self->internal.active_camera = &camera->camera;
}

void ecs_patch_entity(ecs_t *const self, const u32 entity_id, const ecs_cmp_patch_payload_t request)
{
    ecs_componentmanager_patch_entity_components(
        &self->managers.componentmanager, 
        entity_id,
        request
    );
}

void ecs_set_active_commandqueue(ecs_t *const self, commandqueue_t *const commandqueue)
{
    ASSERT(commandqueue);
    self->internal.active_commandqueue = commandqueue;
}

u32 ecs_entity_add(ecs_t *const self, const ecs_componentbundle_t component_config)
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

    ecs_componentmanager_update(&self->managers.componentmanager);

    const ecs_systemmanager_t * const manager = &self->managers.systemmanager;
    for (u8 idx = 0; idx < ECS_SYSTEM_MAX_COUNT; idx++)
    {
        if (!manager->count || !manager->systems[idx].callback)
            continue;

        manager->systems[idx].callback(
            &self->managers.componentmanager,
            (ecs_system_ctx_t) {
                .active_camera = self->internal.active_camera,
                .active_commandqueue = self->internal.active_commandqueue
            }
        );
    }
}

ecs_entity_query_t ecs_entity_query_components(ecs_t *const self, const u32 entity_id, const u32 component_signature)
{
    return ecs_componentmanager__internal_query_components(&self->managers.componentmanager, entity_id, component_signature);
}

void ecs_destroy(ecs_t * const self)
{
    arena_destroy(&self->arena);
}

#endif

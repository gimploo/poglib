#pragma once
#include "../common.h"
#include "poglib/ecs/component.h"
#include "poglib/ecs/component/types.h"
#include "poglib/poggen.h"
#include "poglib/util/glcamera.h"



void ecs_system_camera__internal_update_follow_camera(const vec3f_t camera_offset, glcamera_t *const camera, const ecs_component_entry_t transform_entry)
{
    ASSERT(transform_entry.type == ECS_CMP_TRANSFORM);
    const ecs_component_transform_t *const transform = transform_entry.data;
    ASSERT(transform);

    const vec2f_t delta_rot     = (vec2f_t){ transform->rotation.x, transform->rotation.y };
    const vec3f_t final_offset  = glms_vec3_add(transform->translation, camera_offset);

    glcamera_set(camera, final_offset, delta_rot);
}

void ecs_system_camera__internal_update_free_fly_camera(glcamera_t *const camera, const ecs_component_entry_t transform_entry)
{
    ASSERT(transform_entry.type == ECS_CMP_TRANSFORM);
    const ecs_component_transform_t *const transform = transform_entry.data;
    ASSERT(transform);
    const vec2f_t delta_rot = (vec2f_t){ transform->rotation.x, transform->rotation.y };
    glcamera_set(camera, transform->translation, delta_rot);
}

void ecs_system_camera(ecs_componentmanager_t *const cmp_manager, const ecs_system_ctx_t ctx)
{
    slot_t *pool = slot_get_value(&cmp_manager->componentpool_slots, ECS_CMP_CAMERA_IDX);
    ASSERT(pool);

    slot_iterator(pool, iter) 
    {
        const ecs_component_poolentry_t *const entry = iter;
        ecs_component_camera_t *const camera = (ecs_component_camera_t *)entry->entity_cmpdata;

        switch(camera->mode)
        {
            case ECS_CMP_CAMERA_MODE_FREE_FLY:
                ecs_system_camera__internal_update_free_fly_camera(
                    &camera->camera, 
                    ecs_componentmanager_get_component(cmp_manager, entry->entity_id, ECS_CMP_TRANSFORM)
                );
            break;
            case ECS_CMP_CAMERA_MODE_FOLLOW:
                ecs_system_camera__internal_update_follow_camera(
                    camera->follow.offset,
                    &camera->camera, ecs_componentmanager_get_component(cmp_manager, entry->entity_id, ECS_CMP_TRANSFORM)
                );
            break;
            default: eprint("camera mode not implemented");
        }
    }
}

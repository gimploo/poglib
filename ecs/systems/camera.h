#pragma once
#include "../common.h"
#include "poglib/ecs/component.h"
#include "poglib/ecs/component/types.h"
#include "poglib/util/glcamera.h"

void ecs_system_camera__internal_update_follow_camera(
    ecs_componentmanager_t *const cmp_manager,
    ecs_component_camera_t *const camera,
    const ecs_component_entry_t camera_transform_entry
) {
    const ecs_component_transform_t *const cam_tf       = camera_transform_entry.data;
    const vec2f_t orbit                                 = (vec2f_t){ cam_tf->orientation.x, cam_tf->orientation.y };
    const ecs_component_entry_t player                  = ecs_componentmanager_get_component(cmp_manager, camera->follow.track_entity_id, ECS_CMP_TRANSFORM);
    const ecs_component_transform_t *const player_tf    = player.data;

    glcamera_set(&camera->camera, glms_vec3_add(player_tf->position, camera->follow.offset), orbit);
}

void ecs_system_camera__internal_update_free_fly_camera(glcamera_t *const camera, const ecs_component_entry_t transform_entry)
{
    ASSERT(transform_entry.type == ECS_CMP_TRANSFORM);
    const ecs_component_transform_t *const transform = transform_entry.data;
    ASSERT(transform);
    glcamera_set(
        camera, 
        transform->position, 
        (vec2f_t){ transform->orientation.x, transform->orientation.y }
    );
}

void ecs_system_camera(ecs_componentmanager_t *const cmp_manager, const ecs_system_ctx_t ctx)
{
    slot_t *pool = slot_get_value(&cmp_manager->componentpool_slots, ECS_CMP_CAMERA_IDX);
    ASSERT(pool);

    slot_iterator(pool, iter) 
    {
        const ecs_component_poolentry_t *const entry = iter;
        ecs_component_camera_t *const camera = (ecs_component_camera_t *)entry->entity_cmpdata;

        //NOTE: only updating active camera
        if (!entry->is_active)
            continue;

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
                    cmp_manager,
                    camera, 
                    ecs_componentmanager_get_component(cmp_manager, entry->entity_id, ECS_CMP_TRANSFORM)
                );
            break;
            default: eprint("camera mode not implemented");
        }
    }
}

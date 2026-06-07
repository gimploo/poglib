#pragma once
#include "../common.h"
#include "../component.h"
#include "../component/types.h"
#include "poglib/application.h"
#include "poglib/basic/ds/slot.h"
#include "poglib/external/cglm/struct/vec3.h"
#include <poglib/math.h>

void ecs_system_character_collision_cleanup(ecs_componentmanager_t *cmp_manager);

void ecs_system_character_collision(ecs_componentmanager_t *const cmp_manager, const ecs_system_ctx_t ctx)
{
    /* ---- lazy init static colliders ---- */
    {
        slot_t *sc_pool = slot_get_value(&cmp_manager->componentpool_slots, ECS_CMP_STATIC_COLLIDER_IDX);
        if (sc_pool) {
            slot_iterator(sc_pool, sc_iter)
            {
                ecs_component_poolentry_t *sc_entry = sc_iter;
                if (!sc_entry->is_active) continue;

                ecs_component_static_collider_t *sc = (ecs_component_static_collider_t *)sc_entry->entity_cmpdata;
                if (sc->body_id != 0) continue;

                const ecs_entity_view_t sc_view = ecs_componentmanager__internal_query_components(
                    cmp_manager, sc_entry->entity_id, ECS_CMP_TRANSFORM);
                ecs_component_transform_t *sc_tf = sc_view.entity_cmp_data[ECS_CMP_TRANSFORM_IDX];
                if (!sc_tf) continue;

                switch (sc->type) {
                    case PHYS_COLLIDER_TYPE_CUBE:
                        sc->body_id = physics_sys_jolt_create_box(
                            global_physics_sys_jolt_instance,
                            sc_tf->position,
                            (vec3f_t){ sc->dimension.dim.x / 2.0f, sc->dimension.dim.y / 2.0f, sc->dimension.dim.z / 2.0f },
                            JPH_MotionType_Static,
                            sc->layer,
                            true);
                        break;
                    case PHYS_COLLIDER_TYPE_CAPSULE:
                        sc->body_id = physics_sys_jolt_create_capsule(
                            global_physics_sys_jolt_instance,
                            sc_tf->position,
                            sc->dimension.height / 2.0f,
                            sc->dimension.radius,
                            JPH_MotionType_Static,
                            sc->layer,
                            true);
                        break;
                    default:
                        eprint("unknown static collider type");
                }
            }
        }
    }

    /* ---- character collision ---- */
    slot_t *pool = slot_get_value(&cmp_manager->componentpool_slots, ECS_CMP_CHARACTER_IDX);
    if (!pool) return;

    slot_iterator(pool, iter)
    {
        ecs_component_poolentry_t *entry = iter;
        if (!entry->is_active) continue;

        ecs_component_character_t *collision = (ecs_component_character_t *)entry->entity_cmpdata;

        const ecs_entity_view_t view = ecs_componentmanager__internal_query_components(
            cmp_manager, entry->entity_id, ECS_CMP_TRANSFORM | ECS_CMP_INPUT);
        ecs_component_transform_t *transform = view.entity_cmp_data[ECS_CMP_TRANSFORM_IDX];
        ecs_component_input_t *input = view.entity_cmp_data[ECS_CMP_INPUT_IDX];
        if (!transform) continue;

        /* lazy character creation */
        if (!collision->character) {
            collision->character = physics_sys_jolt_character_create(
                global_physics_sys_jolt_instance,
                global_physics_sys_jolt_instance->physics_system,
                transform->position,
                collision->half_height,
                collision->radius,
                collision->object_layer);
        }

        const vec3f_t desired = input ? input->internal.state.desired_velocity : glms_vec3_zero();
        const bool is_moving = glms_vec3_norm(desired) > 0.001f;

        if (is_moving && ctx.active_camera) {
            vec3f_t world_velocity = glms_vec3_add(
                glms_vec3_scale(ctx.active_camera->direction.front, desired.z),
                glms_vec3_scale(ctx.active_camera->direction.right,  desired.x));
            world_velocity.y = 0;

            const vec3f_t fwd = glms_vec3_normalize((vec3f_t){world_velocity.x, 0, world_velocity.z});
            const f32 yaw = atan2f(fwd.x, fwd.z);
            const versors rot = glms_quatv(yaw, (vec3f_t){0, 1, 0});
            JPH_CharacterVirtual_SetRotation(collision->character, (JPH_Quat *)&rot);

            JPH_CharacterVirtual_SetLinearVelocity(collision->character, (JPH_Vec3 *)&world_velocity);
        } else if (is_moving) {
            JPH_CharacterVirtual_SetLinearVelocity(collision->character, (JPH_Vec3 *)&desired);
        } else {
            JPH_CharacterVirtual_SetLinearVelocity(collision->character, (JPH_Vec3 *)&(vec3f_t){0});
        }

        JPH_ExtendedUpdateSettings ext_settings = {0};
        ext_settings.stickToFloorStepDown = (JPH_Vec3){0, -collision->stick_to_floor_distance, 0};
        ext_settings.walkStairsStepUp = (JPH_Vec3){0, collision->walk_stairs_step, 0};

        JPH_CharacterVirtual_ExtendedUpdate(
            collision->character,
            APPLICATION_UPDATE_FIXED_TIME_STEP,
            &ext_settings,
            collision->object_layer,
            global_physics_sys_jolt_instance->physics_system,
            NULL, NULL);
    }
}

void ecs_system_character_collision_cleanup(ecs_componentmanager_t *cmp_manager)
{
    /* destroy all characters */
    slot_t *col_pool = slot_get_value(&cmp_manager->componentpool_slots, ECS_CMP_CHARACTER_IDX);
    if (col_pool) {
        slot_iterator(col_pool, iter)
        {
            ecs_component_poolentry_t *entry = iter;
            if (!entry->is_active) continue;
            ecs_component_character_t *c = (ecs_component_character_t *)entry->entity_cmpdata;
            if (c->character) {
                physics_sys_jolt_character_destroy(c->character);
                c->character = NULL;
            }
        }
    }

    /* destroy all static collider bodies */
    slot_t *sc_pool = slot_get_value(&cmp_manager->componentpool_slots, ECS_CMP_STATIC_COLLIDER_IDX);
    if (sc_pool) {
        slot_iterator(sc_pool, iter)
        {
            ecs_component_poolentry_t *entry = iter;
            if (!entry->is_active) continue;
            ecs_component_static_collider_t *sc = (ecs_component_static_collider_t *)entry->entity_cmpdata;
            if (sc->body_id != 0) {
                physics_sys_jolt_destroy_body(sc->body_id);
                sc->body_id = 0;
            }
        }
    }
}

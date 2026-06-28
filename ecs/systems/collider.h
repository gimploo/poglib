#pragma once
#include "../common.h"
#include "../component/types.h"
#include "poglib/application.h"
#include "poglib/external/joltc/include/joltc.h"
#include "poglib/physics/jolt-wrapper.h"
#include "../component.h"
#include "poglib/util/workbench/common.h"

void ecs_system_collider(ecs_componentmanager_t *const cmp_manager, const ecs_system_ctx_t ctx)
{
    slot_t *pool = slot_get_value(&cmp_manager->componentpool_slots, ECS_CMP_COLLIDER_IDX);
    slot_iterator(pool, iter)
    {
        ecs_component_poolentry_t *entry = iter;
        if (!entry->is_active) continue;

        ecs_component_collider_t *const collider    = (ecs_component_collider_t *)entry->entity_cmpdata;
        ecs_entity_query_t query                    = ecs_componentmanager__internal_query_components(cmp_manager, entry->entity_id, ECS_CMP_TRANSFORM);
        ecs_component_transform_t *transform        = query.entity_cmp_data[ECS_CMP_TRANSFORM_IDX];

        //HACK: is this good ? - who tf knows - keep this till i get smarter :p
        if (transform->source == ECS_CMP_TRANSFORM_SOURCE_ANIMATION || global_workbench->is_active)
            break;

        switch(collider->motion_type)
        {
            case JPH_MotionType_Kinematic: {
                ASSERT(collider->internal.kinematic_body);

                JPH_CharacterVirtual_SetPosition(collider->internal.kinematic_body, (JPH_Vec3 *)&transform->position);
                JPH_CharacterVirtual_SetRotation(collider->internal.kinematic_body, (JPH_Quat *)&transform->orientation);

                //NOTE: It was initially recommended to use JPH_CharacterVirtualExtendedUpdate to handle gravity but there
                //are so many unknowns in that its better not tackling them now but later.

                const JPH_GroundState ground = JPH_CharacterBase_GetGroundState((JPH_CharacterBase *)collider->internal.kinematic_body);
                if (ground != JPH_GroundState_OnGround) {
                    transform->velocity.y += -9.81f * APPLICATION_UPDATE_FIXED_TIME_STEP;
                } else {
                    transform->velocity = (vec3f_t){0};
                }

                JPH_CharacterVirtual_SetLinearVelocity(collider->internal.kinematic_body, (JPH_Vec3 *)&transform->velocity);
                JPH_CharacterVirtual_Update(
                    collider->internal.kinematic_body, 
                    APPLICATION_UPDATE_FIXED_TIME_STEP, 
                    collider->object_layer_type,
                    global_physics_sys_jolt_instance->physics_system,
                    NULL,
                    NULL
                );
                JPH_CharacterVirtual_GetPosition(collider->internal.kinematic_body, (JPH_Vec3 *)&collider->internal.position);
                JPH_CharacterVirtual_GetRotation(collider->internal.kinematic_body, (JPH_Quat *)&collider->internal.orientation);

                transform->orientation  = collider->internal.orientation;
                transform->position     = collider->internal.position;

            } break;

            case JPH_MotionType_Dynamic:
                ASSERT(collider->internal.body_id);
                JPH_BodyInterface_GetPositionAndRotation(global_physics_sys_jolt_instance->bodyinterface, collider->internal.body_id, (JPH_Vec3 *)&collider->internal.position, (JPH_Quat *)&collider->internal.orientation);
                transform->position     = collider->internal.position;
                transform->orientation  = collider->internal.orientation;
            break;

            case JPH_MotionType_Static: 
                transform->position     = collider->internal.position;
                transform->orientation  = collider->internal.orientation;
            break;

            default: eprint("motion type not accounted for");
        }
    }
}

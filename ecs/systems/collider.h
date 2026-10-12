#pragma once
#include "../common.h"
#include "../component/types.h"
#include "poglib/application.h"
#include "poglib/ecs/systems/simulation.h"
#include "poglib/physics/jolt-wrapper.h"
#include "../component.h"
#include "poglib/poggen.h"
#include "poglib/util/assetmanager.h"
#include "poglib/util/workbench/common.h"

void ecs_system_collider(ecs_componentmanager_t *const cmp_manager, const ecs_system_ctx_t ctx)
{
    slot_t *const pool = slot_get_value(&cmp_manager->componentpool_slots, ECS_CMP_COLLIDER_IDX);
    slot_iterator(pool, iter)
    {
        ecs_component_poolentry_t *entry = (ecs_component_poolentry_t *)iter;
        if (!entry->is_active) continue;

        ecs_component_collider_t *const collider    = (ecs_component_collider_t *)entry->entity_cmpdata;
        ecs_entity_query_t query                    = ecs_componentmanager__internal__query_components(cmp_manager, entry->entity_id, ECS_CMP_TRANSFORM | ECS_CMP_MESH);
        ecs_component_transform_t *transform        = query.entity_cmp_data[ECS_CMP_TRANSFORM_IDX]; ASSERT(transform);

        if (collider->type == ECS_COLLIDER_SOFTBODY) {
            //TODO: Optimize|Thought - this could be cached, also we are enforcing that only a single meshes can be 
            //confiured as softbody during load, which at the moment i dont how i feel about it. I do feel this approach
            //can be improved later.
            ecs_component_mesh_t *mesh = query.entity_cmp_data[ECS_CMP_MESH_IDX]; ASSERT(mesh);
            gpu_asset_t *gpuasset = assetmanager_get_gpu_loaded_asset_async(&global_engine->systems.assets, mesh->asset_id);
            if (!gpuasset) continue;

            gpu_mesh_primitive_t *gpumesh = gpuasset->meshes.data[mesh->mesh_idx]->primitives[mesh->primitive_idx];
            ecs_collider__internal__softbody_update(gpumesh, collider, transform);

            continue;
        }

        if (collider->object_layer_type == POGGEN_RESERVED_OBJECT_LAYER_TYPE) {

            if (!global_workbench->is_active) continue;

            JPH_BodyInterface_SetPositionAndRotation(
                global_joltphysics_instance->bodyinterface,
                collider->body.rigid.body_ids[0], 
                (JPH_Vec3 *)&transform->position,
                (JPH_Quat *)&transform->orientation,
                JPH_Activation_DontActivate
            );
            continue;
        }

        //NOTE: when loading the level, theres a chance the player collider is not created yet, so having this 
        //logic fixes the issue since we start in workbench mode before going to play mode
        if (global_workbench->is_active && collider->motion_type == JPH_MotionType_Kinematic) {
            JPH_CharacterVirtual_SetPosition(collider->body.rigid.kinematicbody, (JPH_Vec3 *)&transform->position);
            JPH_CharacterVirtual_SetRotation(collider->body.rigid.kinematicbody, (JPH_Quat *)&transform->orientation);
        }

        if (global_workbench->is_active) continue;

        switch(collider->motion_type)
        {
            case JPH_MotionType_Kinematic: {
                ASSERT(collider->body.rigid.kinematicbody);

                JPH_CharacterVirtual_SetRotation(collider->body.rigid.kinematicbody, (JPH_Quat *)&transform->orientation);

                //NOTE: handles gravity on the collider (y axis)
                {
                    const JPH_GroundState groundstate = JPH_CharacterBase_GetGroundState((JPH_CharacterBase *)collider->body.rigid.kinematicbody);
                    collider->body.rigid.groundstate = groundstate;

                    if (groundstate == JPH_GroundState_OnGround) {
                        if (transform->velocity.y < -0.1) 
                            transform->velocity.y = 0.f;
                    } else {
                        transform->velocity.y += -9.81f * APPLICATION_UPDATE_FIXED_TIME_STEP;
                    }
                }

                JPH_CharacterVirtual_SetLinearVelocity(collider->body.rigid.kinematicbody, (JPH_Vec3 *)&transform->velocity);

                JPH_CharacterVirtual_ExtendedUpdate(
                    collider->body.rigid.kinematicbody,
                    APPLICATION_UPDATE_FIXED_TIME_STEP,
                    &(JPH_ExtendedUpdateSettings){
                        .stickToFloorStepDown = { 0.f, -0.5f, 0.0f },
                    },
                    collider->object_layer_type,
                    global_joltphysics_instance->physics_system,
                    NULL,
                    NULL
                );

                JPH_CharacterVirtual_GetLinearVelocity(collider->body.rigid.kinematicbody, (JPH_Vec3 *)&transform->velocity);
                JPH_CharacterVirtual_GetPosition(collider->body.rigid.kinematicbody, (JPH_Vec3 *)&collider->internal.position);
                JPH_CharacterVirtual_GetRotation(collider->body.rigid.kinematicbody, (JPH_Quat *)&collider->internal.orientation);

                transform->orientation  = collider->internal.orientation;
                transform->position     = collider->internal.position;
            } break;

            case JPH_MotionType_Dynamic:
                ASSERT(collider->body.rigid.body_ids[0]);
                JPH_BodyInterface_GetPositionAndRotation(
                    global_joltphysics_instance->bodyinterface, 
                    collider->body.rigid.body_ids[0], 
                    (JPH_Vec3 *)&collider->internal.position, 
                    (JPH_Quat *)&collider->internal.orientation
                );
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

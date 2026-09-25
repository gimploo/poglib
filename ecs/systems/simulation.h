#pragma once
#include "poglib/ecs/common.h"
#include "poglib/ecs/component.h"
#include <poglib/poggen.h>

void ecs_system_simulation(ecs_componentmanager_t *const cmp_manager, const ecs_system_ctx_t ctx)
{
    ASSERT(global_engine);

    slot_t *const pool = slot_get_value(&cmp_manager->componentpool_slots, ECS_CMP_SIMULATION_IDX);
    slot_iterator(pool, iter)
    {
        const ecs_component_poolentry_t *const entry    = iter;
        ecs_component_simulation_t *const sim_cmp     = (ecs_component_simulation_t *)entry->entity_cmpdata;

        //NOTE: only supporting cloth simulation for now
        if (!sim_cmp->is_cloth) continue;

        if (!sim_cmp->internal.gpu_loaded_asset) {
            ecs_component_model_t *const model = ecs_componentmanager__internal__query_components(cmp_manager, entry->entity_id, ECS_CMP_MODEL).entity_cmp_data[ECS_CMP_MODEL_IDX];
            sim_cmp->internal.gpu_loaded_asset = assetmanager_get_gpu_loaded_asset_async(&global_engine->systems.assets, model->asset_id);
        }

        gpu_asset_t *const gpu_asset = sim_cmp->internal.gpu_loaded_asset;
        ASSERT(gpu_asset->meshes.count == 1);
        gpu_mesh_t *const gpu_mesh = &gpu_asset->meshes.data[0];
        ASSERT(gpu_mesh->vbo.gpu_mem);

        const ecs_entity_query_t view = ecs_componentmanager__internal__query_components(cmp_manager, entry->entity_id, ECS_CMP_COLLIDER | ECS_CMP_TRANSFORM);
        ecs_component_collider_t *const collider    = view.entity_cmp_data[ECS_CMP_COLLIDER_IDX];
        ecs_component_transform_t *const transform  = view.entity_cmp_data[ECS_CMP_TRANSFORM_IDX];

        ASSERT(transform);
        ASSERT(collider);

        const JPH_Body* body = JPH_PhysicsSystem_GetBodyPtr(global_joltphysics_instance->physics_system, collider->internal.body_id);
        if (!body) {
            logging("unable to get body ptr, skipping this frame");
            continue;
        }

        u32 out_count = 0;
        JPH_Body_GetSoftBodyVertexPositions(body, gpu_mesh->vbo.cpu_write.raw_data, gpu_mesh->vbo.cpu_write.size, &out_count);
        ASSERT(out_count == (gpu_mesh->vbo.cpu_write.size / sizeof(vec3f_t)));

        JPH_Mat4 com_transform;     JPH_Body_GetCenterOfMassTransform(body, &com_transform);
        JPH_Vec3 com_pos;           JPH_Mat4_GetTranslation(&com_transform, &com_pos);
        JPH_Quat com_rot;           JPH_Mat4_GetQuaternion(&com_transform, &com_rot);

        //FIXME: this is pointless do, since now the physics engine straight up provide the worldspace vtx to us,
        //at the moment i dont know how to update the scale, position and orientation of the mesh correctly. Or whether
        //we really need this data anyways in the transform component. Maybe in the future after getting a tiny bit smarter
        //i could probably figure this out.
        transform->position         = *(vec3s *)&com_pos;
        transform->orientation      = *(versors *)&com_rot;

        //NOTE: copy over the prev frame 'gpu read buffer' into actual gpu memory
        memcpy(gpu_mesh->vbo.gpu_mem, gpu_mesh->vbo.gpu_read.raw_data, gpu_mesh->vbo.gpu_read.size);

        //NOTE: update 'gpu read buffer' with latest 'cpu write buffer'
        const vec3s *const vtx_read_list    = (vec3s *)gpu_mesh->vbo.cpu_write.raw_data;
        glvertex3d_t *const vtx_write_list  = (glvertex3d_t *)gpu_mesh->vbo.gpu_read.raw_data;
        for (u32 idx = 0; idx < out_count; idx++) 
            vtx_write_list[idx].pos = vtx_read_list[idx];

    }
}

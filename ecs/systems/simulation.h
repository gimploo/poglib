#pragma once
#include <poglib/poggen.h>

void ecs_collider__internal__softbody_update(gpu_mesh_primitive_t *gpu_mesh_primitive, ecs_component_collider_t *collider, ecs_component_transform_t *transform)
{
    ASSERT(global_engine);
    ASSERT(transform);
    ASSERT(collider);
    ASSERT(gpu_mesh_primitive);

    const JPH_Body* body = JPH_PhysicsSystem_GetBodyPtr(global_joltphysics_instance->physics_system, collider->body.soft.bodyid);
    if (!body) {
        logging("unable to get body ptr, skipping this frame");
        return;
    }

    u32 out_count = 0;
    buffer_t scrap_buffer = runtime_scratchbuffer();
    JPH_Body_GetSoftBodyVertexPositions(body, (JPH_Vec3 *)scrap_buffer.raw_data, scrap_buffer.size, &out_count);

    u8 *gpu_mem = gpu_mesh_primitive->vbo.gpu_mem;    ASSERT(gpu_mem);
    u16 vtx_stride = gpu_mesh_primitive->vtx_stride;  ASSERT(vtx_stride);
    for(u32 vtx_idx = 0; vtx_idx < out_count; vtx_idx++)
    {
        *(vec3s *)(gpu_mem + (vtx_idx * vtx_stride)) = ((vec3s *)scrap_buffer.raw_data)[vtx_idx];
    }
}

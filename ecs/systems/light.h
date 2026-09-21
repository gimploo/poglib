#pragma once
#include "../component/types.h"
#include "poglib/ecs/common.h"
#include "poglib/ecs/component.h"
#include "poglib/gfx/gl/ubo.h"

// Explicitly aligned struct matching GLSL std140 layout
typedef struct {
    f32 position[4]; // x, y, z, pad
    f32 color[4];    // r, g, b, intensity
    f32 params[4];   // constant, linear, quadratic, radius
} gl_pointlight_t;

// Maximum array size
#define MAX_LIGHTS 16

//NOTE: this needs to be 16 bytes alligned at all times
typedef struct {
    gl_pointlight_t lights[MAX_LIGHTS];
    alignas(16) struct { 
        vec3s global_ambient;
        f32 global_specular_strength;
    };
    u32 numLights;
} gllightbuffer_t ;

void ecs_system_light(ecs_componentmanager_t *const cmp_manager, const ecs_system_ctx_t ctx)
{
    ASSERT(ctx.ubo);

    slot_t *const pool = slot_get_value(&cmp_manager->componentpool_slots, ECS_CMP_LIGHT_IDX);
    gllightbuffer_t bufferData = {
        .numLights = pool->len,
        .global_ambient = (vec3s){ 0.1f, 0.1f, 0.1f },
        .global_specular_strength = 0.5f,
        .lights = {0},
    };
    slot_iterator(pool, iter)
    {
        const u64 slot_index = (u64)slot_iterator_index;
        ecs_component_poolentry_t *const entry = iter;
        ecs_component_light_t *light = (ecs_component_light_t *)entry->entity_cmpdata;

        ecs_component_transform_t *transform = ecs_componentmanager__internal__query_components(
            cmp_manager, entry->entity_id, ECS_CMP_TRANSFORM
        ).entity_cmp_data[ECS_CMP_TRANSFORM_IDX];

        if (!entry->is_active) continue;

        // Populate positions
        bufferData.lights[slot_index].position[0] = transform->position.x;
        bufferData.lights[slot_index].position[1] = transform->position.y;
        bufferData.lights[slot_index].position[2] = transform->position.z;

        // Populate color + intensity
        bufferData.lights[slot_index].color[0] = light->color.r;
        bufferData.lights[slot_index].color[1] = light->color.g;
        bufferData.lights[slot_index].color[2] = light->color.b;
        bufferData.lights[slot_index].color[3] = light->color.a;

        // Attenuation parameters
        bufferData.lights[slot_index].params[0] = light->constant;
        bufferData.lights[slot_index].params[1] = light->linear;
        bufferData.lights[slot_index].params[2] = light->quadratic;
        bufferData.lights[slot_index].params[3] = light->radius;
    }

    glubo_upload(
        ctx.ubo, 
        (buffer_t){
            .size = sizeof(bufferData),
            .raw_data = &bufferData,
        }
    );
}

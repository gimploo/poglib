#pragma once
#include "poglib/ecs/common.h"
#include "poglib/ecs/component.h"
#include "poglib/ecs/component/material_uniform_resolver.h"
#include "poglib/poggen.h"

//TODO: limit how many times this is ran, since if the scene only includes static meshes and the player
//not moving, doing this once is enough - i think.
//also due to the size limit of how much it covers it also needs to get updated too to cover areas that 
//were not covered before relative to the player position

void ecs_system_directional_light(ecs_componentmanager_t *const cmp_manager, const ecs_system_ctx_t ctx)
{
    slot_t *const pool = slot_get_value(&cmp_manager->componentpool_slots, ECS_CMP_DIRECTIONAL_LIGHT_IDX);
    if (!pool->len) return;

    if (!ctx.renderqueue->buckets.len) return;

    GL_CHECK(glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT));
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, ctx.fbo.id));

    slot_iterator(pool, iter)
    {
        ecs_component_poolentry_t *const entry = (ecs_component_poolentry_t *)iter;
        ecs_component_directional_light_t *const directional_light = (ecs_component_directional_light_t *)entry->entity_cmpdata;

        GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, directional_light->internal.shadow_texid, 0));
        ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
        GL_CHECK(glClear(GL_DEPTH_BUFFER_BIT));

        //TODO: have the direcitonal light's position change, as the player moves around. such that the 
        //the shadow map does not go stale.

        ecs_component_transform_t *transform = ecs_componentmanager__internal__query_components(
            cmp_manager, entry->entity_id, ECS_CMP_TRANSFORM
        ).entity_cmp_data[ECS_CMP_TRANSFORM_IDX];

        transform->position.y = directional_light->radius;

        const vec3s light_position      = transform->position;
        const mat4s lightview           = glms_look(light_position, directional_light->direction, GL_CAMERA_DIRECTION_UP); 
        const mat4s lightspace_matrix   = glms_mat4_mul(directional_light->projection, lightview);

        //TODO: use depth variants for each binded shader for the pass - bot recommended to use #define macros 
        //for different variants and inject the defines into string buffer 
        //example -
        //    #version 450 core
        //    layout(location = 0) in vec3 a_Position;
        //    #ifdef ALPHA_TEST
        //    layout(location = 1) in vec2 a_TexCoord;
        //    out vec2 v_TexCoord;
        //    #endif
        //    #ifdef USE_SKINNING
        //    layout(location = 2) in vec4 a_Weights;
        //    layout(location = 3) in uvec4 a_BoneIDs;
        //    layout(std140, binding = 1) uniform BoneBlock {
        //        mat4 u_BoneMatrices[128];
        //    };
        //    #endif


        GL_CHECK(glEnable(GL_POLYGON_OFFSET_FILL));

        //NOTE: Practical Rule of Thumb for Values
        //---
        //* (1.0f, 1.0f) to (2.0f, 4.0f): Standard starting range for typical 24-bit or 32-bit floating-point depth maps (GL_DEPTH_COMPONENT24 or GL_DEPTH_COMPONENT32F).
        //* If you see Peter Panning (shadows detaching from the base of objects): Your offsets are too high. Reduce units down to 1.0f or 2.0f.
        //* If you see Shadow Acne (stitching/stripes across geometry): Your offsets are too low. Increase factor to 2.5f or 3.0f for sloped surfaces.
        //ex -  slope_factor = 2.0, units = 4.0 (adjust based on scene scale)
        const f32 factor = 2.0f; 
        const f32 units = 4.0f;
        GL_CHECK(glPolygonOffset(factor, units));

        renderqueue_dispatch_depthpass(
            ctx.renderqueue, 
            true, 
            (gluniforms_t) {
                .count = 2,
                .data = {
                    [0] = {
                        .name = ECS_UNIFORM_SUPPORTED_NAME_LOOKUP[ECS_UNIFORM_CAMERA_PROJECTION],
                        .value.mat4 = directional_light->projection
                    },
                    [1] = {
                        .name = ECS_UNIFORM_SUPPORTED_NAME_LOOKUP[ECS_UNIFORM_CAMERA_VIEW],
                        .value.mat4 = lightview
                    },
                    //TODO: this shouldnt be here, having a depth varient for each shader would remove this
                    //from here
                }
            }
        );
        GL_CHECK(glDisable(GL_POLYGON_OFFSET_FILL));

        mat4s shadowmap_transform   = glms_mat4_identity();
        shadowmap_transform         = glms_translate(shadowmap_transform, (vec3s){{0.5f, 0.5f, 0.5f}});
        shadowmap_transform         = glms_scale(shadowmap_transform, (vec3s){{0.5f, 0.5f, 0.5f}});

        directional_light->internal.shadowmap_matrix = glms_mat4_mul(shadowmap_transform, lightspace_matrix);
    }
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    GL_CHECK(glViewport(0, 0, global_window->width, global_window->height));

    renderqueue_flush(ctx.renderqueue);
}


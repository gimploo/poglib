#pragma once
#include "../component/types.h"
#include "poglib/ecs/common.h"
#include "poglib/ecs/component.h"

void ecs_system_shadow(ecs_componentmanager_t *const cmp_manager, const ecs_system_ctx_t ctx)
{
    slot_t *const pool = slot_get_value(&cmp_manager->componentpool_slots, ECS_CMP_LIGHT_IDX);
    if (!pool->len) return;

    slot_iterator(pool, iter)
    {
        ecs_component_poolentry_t *const entry = iter;
        ecs_component_light_t *const light = (ecs_component_light_t *)entry->entity_cmpdata;
        const ecs_component_transform_t *const light_transform = ecs_componentmanager__internal__query_components(
            cmp_manager, entry->entity_id, ECS_CMP_TRANSFORM
        ).entity_cmp_data[ECS_CMP_TRANSFORM_IDX];
        ASSERT(light_transform);

        gldepthbuffer_t cubemap = gldepthbuffer_cubemap_init(1024, 1024);

        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, cubemap.fbo_id));
        GL_CHECK(glViewport(0, 0, cubemap.width, cubemap.height));

        for (u8 face_idx = 0; face_idx < 6; ++face_idx)
        {
            // Attach specific face to FBO
            GL_CHECK(glFramebufferTexture2D(
                GL_FRAMEBUFFER, 
                GL_DEPTH_ATTACHMENT, 
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + face_idx, 
                cubemap.tex_id, 
                0
            ));

            GL_CHECK(glClear(GL_DEPTH_BUFFER_BIT));

            //TODO: render logic in here

        }
        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    }
}

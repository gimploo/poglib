#pragma once
#include <poglib/poggen.h>
#include "poglib/ecs/common.h"
#include "poglib/ecs/component.h"
#include "poglib/ecs/component/types.h"
#include "poglib/external/cglm/struct/mat4.h"
#include "poglib/gfx/model/assimp.h"
#include "poglib/math/la.h"
#include "poglib/pipeline/render/common.h"
#include "poglib/pipeline/render/render_queue.h"
#include "poglib/poggen.h"
#include "poglib/util/asset.h"
#include "poglib/util/assetmanager.h"
#include "poglib/util/glcamera.h"

void ecs_system_render_model(ecs_componentmanager_t *const cmp_manager, const ecs_system_ctx_t ctx)
{
    ASSERT(global_engine);
    ASSERT(ctx.active_camera);

    slot_t *primary_pool = slot_get_value(&cmp_manager->componentpool_slots, ECS_CMP_MODEL_IDX);

    slot_iterator(primary_pool, ITER)
    {
        const ecs_component_poolentry_t * const entry = ITER;
        ecs_component_model_t *cmp_model = (ecs_component_model_t *)entry->entity_cmpdata;

        if (!cmp_model->internal.model) {
            cmp_model->internal.model = (glmodel_t *)assetmanager_get_assetresource(
                &global_engine->systems.assets,
                ASSET_TYPE_MODEL,
                (cmp_model)->asset_id
            );
        }

        if (!cmp_model->internal.model) continue;

        const gpu_asset_t *gpu_loaded_asset = (gpu_asset_t *)assetmanager_get_gpu_loaded_asset_async(
            &global_engine->systems.assets,
            cmp_model->asset_id
        );
        if (!gpu_loaded_asset) continue;

        const u32 entity_id = entry->entity_id;
        const ecs_entity_query_t view = ecs_componentmanager__internal_query_components(
                cmp_manager,
                entity_id,
                ECS_CMP_MATERIAL | ECS_CMP_TRANSFORM);
        const ecs_component_material_t *material    = view.entity_cmp_data[ECS_CMP_MATERIAL_IDX];
        const ecs_component_transform_t *transform  = view.entity_cmp_data[ECS_CMP_TRANSFORM_IDX];
        const glshader_t *shader                    = assetmanager_get_assetresource(&global_engine->systems.assets, ASSET_TYPE_GLSL_SHADER, material->shader.asset_id);

        ASSERT(material);
        ASSERT(shader);
        ASSERT(transform);

        const glmodel_t *model = cmp_model->internal.model;
        gltexturelist_t model_textures = glmodel_get_texuturelist(model);

        const matrix4f_t proj = glms_perspective(
            radians(45), global_engine->handle.app->window.aspect_ratio, 1.0f, 1000.0f);
        const matrix4f_t view_mat = glcamera_getview(ctx.active_camera);
        const matrix4f_t model_mat = glms_mat4_mul(
            glms_translate_make(transform->position),
            glms_mat4_mul(glms_quat_mat4(transform->orientation), glms_scale_make(transform->scale)));

        ASSERT(gpu_loaded_asset->meshes.count == model->meshes.len);
        for (u8 idx = 0; idx < model->meshes.len; idx++)
        {
            const vec4f_t mesh_color = *(vec4f_t *)list_get_value(&model->colors, idx);
            const u32 bone_count = model->transforms[idx].len;
            matrix4f_t *bone_data = (matrix4f_t *)model->transforms[idx].data;

            rendercommand_t cmd = {
                .mesh = &gpu_loaded_asset->meshes.data[idx],
                .draw_mode = RENDER_COMMAND_DRAW_MODE_TRIANGLE,
                .enable_wireframe = false,
                .instance = {0},
                .material = {
                    .texture = model_textures,
                    .shader = {
                        .data = shader,
                        .uniforms = {0},
                    }
                }
            };

            if (material->textures.count) {
                for (u8 t = 0; t < material->textures.count; t++) {
                    if (cmd.material.texture.count < MAX_SUPPORTED_TEXTURE_COUNT_PER_DRAW_CALL) {
                        cmd.material.texture.items[cmd.material.texture.count++] = material->textures.items[t];
                    }
                }
            }

            gluniforms_t *u = &cmd.material.shader.uniforms;

            u->data[u->count].name = str("view");
            u->data[u->count].value.mat4 = view_mat;
            u->count++;

            u->data[u->count].name = str("projection");
            u->data[u->count].value.mat4 = proj;
            u->count++;

            u->data[u->count].name = str("transform");
            u->data[u->count].value.mat4 = model_mat;
            u->count++;

            u->data[u->count].name = str("material.color");
            u->data[u->count].value.vec4 = mesh_color;
            u->count++;

            if (bone_count) {
                u->data[u->count].name = str("uBones");
                u->data[u->count].value.mat4s.count = bone_count;
                u->data[u->count].value.mat4s.data = bone_data;
                u->count++;
            }

            for (u8 i = 0; i < material->shader.uniforms.count; i++)
                u->data[u->count++] = material->shader.uniforms.data[i];

            renderqueue_pass_command(&global_engine->systems.renderqueue, cmd);
        }
    }
}

#pragma once
#include <poglib/poggen.h>
#include "poglib/ecs/common.h"
#include "poglib/ecs/component.h"
#include "poglib/ecs/component/material_uniform_resolver.h"
#include "poglib/ecs/component/types.h"
#include "poglib/gfx/model/assimp.h"
#include "poglib/pipeline/render/render_queue.h"
#include "poglib/poggen.h"
#include "poglib/util/asset.h"
#include "poglib/util/assetmanager.h"

void ecs_system_render_model(ecs_componentmanager_t *const cmp_manager, const ecs_system_ctx_t ctx)
{
    ASSERT(global_engine);
    ASSERT(ctx.active_camera);

    slot_t *const primary_pool = slot_get_value(&cmp_manager->componentpool_slots, ECS_CMP_MODEL_IDX);

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
        const ecs_entity_query_t view = ecs_componentmanager__internal__query_components(
                cmp_manager,
                entity_id,
                ECS_CMP_MATERIAL | ECS_CMP_TRANSFORM);
        ecs_component_material_t *material          = view.entity_cmp_data[ECS_CMP_MATERIAL_IDX];
        const ecs_component_transform_t *transform  = view.entity_cmp_data[ECS_CMP_TRANSFORM_IDX];
        const glshader_t *shader                    = assetmanager_get_assetresource(&global_engine->systems.assets, ASSET_TYPE_GLSL_SHADER, material->shader_asset_id);

        ASSERT(shader);
        ASSERT(transform);

        const glmodel_t *model = cmp_model->internal.model;
        ASSERT(gpu_loaded_asset->meshes.count == model->meshes.len);
        const gltexturelist_t textures = glmodel_get_texuturelist(model);

        for (u8 idx = 0; idx < model->meshes.len; idx++)
        {
            gluniforms_t mesh_uniforms = material->internal.uniform_values;
            ecs_material_resolve_per_mesh_uniforms(cmp_model, shader, idx, &mesh_uniforms);

            rendercommand_t cmd = {
                .vtx.data.mesh = &gpu_loaded_asset->meshes.data[idx],
                .enable_wireframe = false,
                .instance = {0},
                .material = {
                    .texture = textures,
                    .shader = {
                        .data = (glshader_t *)shader,
                        .uniforms = mesh_uniforms,
                    }
                }
            };
            renderqueue_pass_command(&global_engine->systems.renderqueue, cmd);
        }
    }
}

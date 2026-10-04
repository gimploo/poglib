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

INTERNAL void ecs_system_render__internal__scene_instanced_models(
    ecs_component_material_t *material,
    ecs_component_transform_t *transform,
    u32 entity_id,
    u32 model_asset_id
) {
    const gpu_asset_t *gpu_loaded_asset = (gpu_asset_t *)assetmanager_get_gpu_loaded_asset_async(
        &global_engine->systems.assets,
        model_asset_id
    );
    ASSERT(gpu_loaded_asset);

    const glshader_t *shader  = assetmanager_get_assetresource(&global_engine->systems.assets, ASSET_TYPE_GLSL_SHADER, material->shader_asset_id);
    ASSERT(shader);

    const bool is_editor_selected = global_workbench->editor.current_selected_entity_id == entity_id;

    for (u8 idx = 0; idx < gpu_loaded_asset->meshes.count; idx++)
    {
        rendercommand_t cmd = {
            .vtx.data.mesh = &gpu_loaded_asset->meshes.data[idx],
            .instance = {
                .size = sizeof(rendercommand_instance_primitive_mesh_t),
                .raw_data = &(rendercommand_instance_primitive_mesh_t) {
                    .translation = (vec4f_t) { transform->position.x, transform->position.y, transform->position.z, 0.f },
                    .scale = (vec4f_t) { transform->scale.x, transform->scale.y,  transform->scale.z,  0.f },
                    .orientation = *(vec4f_t *)&transform->orientation,
                    .color = is_editor_selected ? COLOR_RED : material->color,
                    .uv = {0},
                },
            },
            .enable_wireframe = false,
            .material = {
                .texture = {0},
                .shader = {
                    .data = (glshader_t *)shader,
                    .uniforms = material->internal.uniform_values,
                }
            }
        };
        renderqueue_pass_command(&global_engine->systems.renderqueue, cmd);
    }
}


void ecs_system_render_model(ecs_componentmanager_t *const cmp_manager, const ecs_system_ctx_t ctx)
{
    ASSERT(global_engine);
    ASSERT(ctx.active_camera);

    slot_t *const primary_pool = slot_get_value(&cmp_manager->componentpool_slots, ECS_CMP_MODEL_IDX);

    slot_iterator(primary_pool, ITER)
    {
        const ecs_component_poolentry_t * const entry = ITER;
        ecs_component_model_t *cmp_model = (ecs_component_model_t *)entry->entity_cmpdata;

        const ecs_entity_query_t view = ecs_componentmanager__internal__query_components(
            cmp_manager,
            entry->entity_id,
            ECS_CMP_MATERIAL | ECS_CMP_TRANSFORM
        );

        if (cmp_model->is_scene_instanced) {
            ecs_component_material_t *material = view.entity_cmp_data[ECS_CMP_MATERIAL_IDX];
            ecs_component_transform_t *transform = view.entity_cmp_data[ECS_CMP_TRANSFORM_IDX];
            ecs_system_render__internal__scene_instanced_models(material, transform, entry->entity_id, cmp_model->asset_id);
            continue;
        }

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

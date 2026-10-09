#pragma once
#include <poglib/poggen.h>
#include "poglib/ecs/common.h"
#include "poglib/ecs/component.h"
#include "poglib/ecs/component/material_texture_resolver.h"
#include "poglib/ecs/component/material_uniform_resolver.h"
#include "poglib/ecs/component/types.h"
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
        const ecs_component_poolentry_t *const entry = (void *)ITER;
        ecs_component_model_t *cmp_model = (ecs_component_model_t *)entry->entity_cmpdata;

        const ecs_entity_query_t view = ecs_componentmanager__internal__query_components(
            cmp_manager,
            entry->entity_id,
            ECS_CMP_MATERIAL | ECS_CMP_TRANSFORM
        );

        ecs_component_material_t *material = view.entity_cmp_data[ECS_CMP_MATERIAL_IDX];
        ecs_component_transform_t *transform = view.entity_cmp_data[ECS_CMP_TRANSFORM_IDX];

        const gpu_asset_t *gpu_loaded_asset = (gpu_asset_t *)assetmanager_get_gpu_loaded_asset_async(
            &global_engine->systems.assets,
            cmp_model->asset_id
        );
        if(!gpu_loaded_asset) continue;

        const glshader_t *shader  = assetmanager_get_assetresource(&global_engine->systems.assets, ASSET_TYPE_GLSL_SHADER, material->shader_asset_id);
        ASSERT(shader);

        const bool is_editor_selected = global_workbench->editor.current_selected_entity_id == entry->entity_id;

        for (u32 mesh_idx = 0; mesh_idx < gpu_loaded_asset->meshes.count; mesh_idx++)
        {
            renderqueue_pass_command(&global_engine->systems.renderqueue,
                (rendercommand_t ){
                    .vtx.data = {
                        .mesh = &gpu_loaded_asset->meshes.data[mesh_idx],
                    },
                    .instance = {
                        .size = sizeof(rendercommand_instance_primitive_mesh_t),
                        .raw_data = &(rendercommand_instance_primitive_mesh_t) {
                            .translation = (vec4f_t) { transform->position.x, transform->position.y, transform->position.z, 0.f },
                            .scale = (vec4f_t) { transform->scale.x, transform->scale.y,  transform->scale.z,  0.f },
                            .orientation = *(vec4f_t *)&transform->orientation,
                            .color = is_editor_selected ? COLOR_RED : ecs_material_get_basecolor_for_mesh(material, mesh_idx),
                        },
                    },
                    .enable_wireframe = false,
                    .material = {
                        .texture = ecs_material_get_texturelist_for_mesh(material, mesh_idx),
                        .shader = {
                            .data = (glshader_t *)shader,
                            .uniforms = ecs_material_resolve_per_mesh_uniforms(material, shader, mesh_idx),
                        }
                    }
                }
            );
        }
    }
}

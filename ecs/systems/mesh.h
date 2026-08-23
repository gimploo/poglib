#pragma once
#include <poglib/poggen.h>
#include "poglib/ecs/common.h"
#include "poglib/ecs/component.h"
#include "poglib/ecs/component/types.h"
#include "poglib/math/la.h"
#include "poglib/pipeline/render/render_queue.h"
#include "poglib/poggen.h"
#include "poglib/util/asset.h"
#include "poglib/util/assetmanager.h"
#include "poglib/util/workbench/common.h"

void ecs_system_render_mesh(ecs_componentmanager_t *const cmp_manager, const ecs_system_ctx_t ctx)
{
    ASSERT(global_engine);
    ASSERT(ctx.active_camera);

    slot_t *primary_pool = slot_get_value(&cmp_manager->componentpool_slots, ECS_CMP_MESH_IDX);

    slot_iterator(primary_pool, ITER)
    {
        const ecs_component_poolentry_t * const entry = ITER;
        const ecs_component_mesh_t *const mesh = (ecs_component_mesh_t *)entry->entity_cmpdata;
        const gpu_asset_t *gpu_loaded_asset = (gpu_asset_t *)assetmanager_get_gpu_loaded_asset_async(
            &global_engine->systems.assets, 
            mesh->asset_id
        );
        if (!gpu_loaded_asset) {
            continue;
        }

        const bool is_editor_selected = global_workbench->editor.current_selected_entity_id == entry->entity_id;

        const ecs_entity_query_t view = ecs_componentmanager__internal__query_components(
            cmp_manager, 
            entry->entity_id, 
            ECS_CMP_MATERIAL | ECS_CMP_TRANSFORM | ECS_CMP_SPRITE);

        ecs_component_material_t *material = view.entity_cmp_data[ECS_CMP_MATERIAL_IDX];
        ASSERT(material);

        const ecs_component_transform_t *transform = view.entity_cmp_data[ECS_CMP_TRANSFORM_IDX];
        ASSERT(transform);

        const glshader_t *shader = assetmanager_get_assetresource(&global_engine->systems.assets, ASSET_TYPE_GLSL_SHADER, material->shader_asset_id);
        ASSERT(shader);

        gltexturelist_t textures = {0};

        //NOTE: resolve texture & uv
        const ecs_component_sprite_t *sprite_cmp = view.entity_cmp_data[ECS_CMP_SPRITE_IDX];
        box_t uv = {0};
        if (sprite_cmp)
        {
            spriteatlas_t *const atlas = (spriteatlas_t *)assetmanager_get_assetresource(
                &global_engine->systems.assets, 
                ASSET_TYPE_TEXTURE_SPRITE_ATLAS, 
                sprite_cmp->spritesheet_asset_id
            );
            uv = spriteatlas_get_sprite(atlas, sprite_cmp->sprite_idx);
            textures.items[textures.count++] = (gltextureitem_t){
                .type = GL_TEXTURE_TYPE_NORMAL,
                .source.normal_texture = &atlas->texture
            };
        }

        //NOTE: resolve textures
        for (u32 texidx = 0; texidx < (material && material->textures.count); texidx++)
        {
            gltexture2d_t *const texture = (gltexture2d_t *)assetmanager_get_assetresource(
                &global_engine->systems.assets, 
                ASSET_TYPE_TEXTURE,
                material->textures.asset_ids[texidx] 
            );
            textures.items[textures.count++] = (gltextureitem_t){
                .type = GL_TEXTURE_TYPE_NORMAL,
                .source.normal_texture = texture
            };
        }

        //NOTE: backwards compatabitlity for color in material
        {
            material->color = material->color.a == 0.f ? COLOR_WHITE : material->color;
        }

        const rendercommand_t command = {
            .vtx = {
                .data.mesh = &gpu_loaded_asset->meshes.data[mesh->mesh_idx],
            },
            .enable_wireframe = false,
            .instance = {
                .raw_data = &(rendercommand_instance_primitive_mesh_t) {
                    .translation = (vec4f_t) { transform->position.x, transform->position.y, transform->position.z, 0.f },
                    .scale = (vec4f_t) { transform->scale.x, transform->scale.y,  transform->scale.z,  0.f },
                    .orientation = *(vec4f_t *)&transform->orientation,
                    .color = is_editor_selected ? COLOR_RED : material->color,
                    .uv = uv,
                },
                .size = sizeof(rendercommand_instance_primitive_mesh_t)
            },
            .material = {
                .texture = textures,
                .shader = {
                    .data = shader,
                    .uniforms = material->internal.uniform_values,
                }
            }
        };

        renderqueue_pass_command(&global_engine->systems.renderqueue, command);
    }
}

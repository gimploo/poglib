#pragma once
#include <poglib/poggen.h>
#include "poglib/basic/common.h"
#include "poglib/ecs/common.h"
#include "poglib/ecs/component.h"
#include "poglib/ecs/component/types.h"
#include "poglib/math/la.h"
#include "poglib/pipeline/render/render_queue.h"
#include "poglib/poggen.h"
#include "poglib/util/asset.h"
#include "poglib/util/assetmanager.h"
#include "poglib/util/workbench.h"

void ecs_system_render_mesh(ecs_componentmanager_t *const cmp_manager, const ecs_system_ctx_t ctx)
{
    ASSERT(global_engine);
    ASSERT(ctx.active_camera);

    slot_t *primary_pool = slot_get_value(&cmp_manager->componentpool_slots, ECS_CMP_MESH_IDX);

    slot_iterator(primary_pool, ITER)
    {
        const ecs_component_poolentry_t * const entry = ITER;
        const u32 assetid = ((ecs_component_model_t *)entry->entity_cmpdata)->asset_id;
        const gpu_asset_t *gpu_loaded_asset = (gpu_asset_t *)assetmanager_get_gpu_loaded_asset_async(
            &global_engine->systems.assets, 
            assetid
        );
        if (!gpu_loaded_asset) {
            continue;
        }

        const u32 entity_id = entry->entity_id;
        const ecs_entity_query_t view = ecs_componentmanager__internal_query_components(
                cmp_manager, 
                entity_id, 
                ECS_CMP_MATERIAL | ECS_CMP_TRANSFORM);
        const ecs_component_material_t *material    = view.entity_cmp_data[ECS_CMP_MATERIAL_IDX];
        const ecs_component_transform_t *transform  = view.entity_cmp_data[ECS_CMP_TRANSFORM_IDX];
        const glshader_t *shader                    = assetmanager_get_assetresource(&global_engine->systems.assets, ASSET_TYPE_GLSL_SHADER, material->shader_asset_id);

        ASSERT(material);
        ASSERT(shader);
        ASSERT(transform);

        spriteatlas_t *atlas = (spriteatlas_t *)assetmanager_get_assetresource(
                &global_engine->systems.assets, ASSET_TYPE_TEXTURE_SPRITE_ATLAS, global_workbench->primitives.atlas_id);

        const matrix4f_t perspective_projection = glms_perspective(
            radians(45), 
            global_engine->handle.app->window.aspect_ratio, 
            1.0f, 1000.0f
        );

        const rendercommand_instance_t instance = {
            .translation = (vec4f_t) { transform->position.x, transform->position.y, transform->position.z, 0.f },
            .scale = (vec4f_t) { transform->scale.x, transform->scale.y,  transform->scale.z,  0.f },
            .orientation = *(vec4f_t *)&transform->orientation,
            .color = COLOR_WHITE,
            .uv = spriteatlas_get_sprite(atlas, PROTOTYPE_SPRITE_CHECKERED_GRAY),
        };

        rendercommand_t command = {
            .mesh = gpu_loaded_asset->meshes.data,
            .draw_mode = RENDER_COMMAND_DRAW_MODE_TRIANGLE,
            .enable_wireframe = false,
            .instance = {
                .raw_data = {0},
                .size = sizeof(rendercommand_instance_t)
            },
            .material = {
                .textures = {
                    .count = 1,
                    .ids = {
                        [0] = atlas->texture.id,
                    }
                },
                .shader = {
                    .data = shader,
                    .uniforms = {
                        .count = 2,
                        .data = {
                            [0] = {
                                .name = str("projection"),
                                .value = perspective_projection
                            },
                            [1] = {
                                .name = str("view"),
                                .value = glcamera_getview(ctx.active_camera),
                            }
                        }
                    }
                }
            }
        };

        memcpy(&command.instance.raw_data, &instance, sizeof(instance));
        renderqueue_pass_command(&global_engine->systems.renderqueue, command);
    }
}


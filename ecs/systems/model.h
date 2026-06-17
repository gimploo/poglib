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
    ASSERT(global_poggen);
    ASSERT(ctx.active_camera);

    slot_t *primary_pool = slot_get_value(&cmp_manager->componentpool_slots, ECS_CMP_MODEL_IDX);

    slot_iterator(primary_pool, ITER)
    {
        const ecs_component_poolentry_t * const entry = ITER;
        ecs_component_model_t *cmp_model = (ecs_component_model_t *)entry->entity_cmpdata;

        if (!cmp_model->internal.model) {
            cmp_model->internal.model = (glmodel_t *)assetmanager_get_assetresource(
                &global_poggen->systems.assets, 
                ASSET_TYPE_MODEL,
                (cmp_model)->asset_id
            );
        }


        if (!cmp_model->internal.model) {
            continue;
        }

        const gpu_asset_t *gpu_loaded_asset = (gpu_asset_t *)assetmanager_get_gpu_loaded_asset_async(
            &global_poggen->systems.assets, 
            cmp_model->asset_id
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
        const glshader_t *shader                    = assetmanager_get_assetresource(&global_poggen->systems.assets, ASSET_TYPE_GLSL_SHADER, material->shader_asset_id);

        ASSERT(material);
        ASSERT(shader);
        ASSERT(transform);

        const matrix4f_t perspective_projection = glms_perspective(
            radians(45), 
            global_poggen->handle.app->window.aspect_ratio, 
            1.0f, 1000.0f
        );

        const glmodel_t *model = cmp_model->internal.model;
        ASSERT(gpu_loaded_asset->meshes.count == model->meshes.len);
        for (u8 idx = 0; idx < model->meshes.len; idx++)
        {
            renderqueue_pass_command(
                &global_poggen->systems.renderqueue, 
                (rendercommand_t) {
                    .mesh = &gpu_loaded_asset->meshes.data[idx],
                    .draw_mode = RENDER_COMMAND_DRAW_MODE_TRIANGLE,
                    .enable_wireframe = false,
                    .instance = {0},
                    .material = {
                        .textures = {0},
                        .shader = {
                            .data = shader,
                            .uniforms = {
                                .count = 8,
                                .data = {
                                    [0] = {
                                        .name = str_lit("view"),
                                        .value = glcamera_getview(ctx.active_camera),
                                    },
                                    [1] = {
                                        .name = str_lit("projection"), 
                                        .value = perspective_projection
                                    },
                                    [2] = {
                                        .name = str_lit("transform"),
                                        .value = glms_mat4_mul(
                                            glms_translate_make(transform->position),
                                            glms_mat4_mul(
                                                glms_quat_mat4(transform->orientation),
                                                glms_scale_make(transform->scale)
                                            )
                                        )
                                    },
                                    [3] = {
                                        .name = str_lit("material.color"),
                                        .value.vec4 = *(vec4f_t *)list_get_value(&model->colors, idx)
                                    },
                                    [4] = {
                                        .name = str_lit("uBones"), 
                                        .value.mat4s = {
                                            .count = model->transforms[idx].len, 
                                            .data = (matrix4f_t *)model->transforms[idx].data
                                        }
                                    },
                                    [5] = {
                                        .name = str_lit("light.color"),
                                        .value.vec4 = COLOR_WHITE
                                    },
                                    [6] = {
                                        .name = str_lit("light.ambient"),
                                        .value.f32 = 1.0f
                                    },
                                    [7] = {
                                        .name = str_lit("light.position"),
                                        .value.vec3 = vec3f(1.0f)
                                    }
                                }
                            }
                        }
                    }
                }
            );
        }
    }
}


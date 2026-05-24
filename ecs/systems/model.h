#pragma once
#include "poglib/ecs/common.h"
#include "poglib/ecs/component.h"
#include "poglib/ecs/component/types.h"
#include "poglib/gfx/model/assimp.h"
#include "poglib/math/la.h"
#include "poglib/pipeline/render/common.h"
#include "poglib/pipeline/render/render_queue.h"
#include "poglib/poggen.h"
#include "poglib/util/asset.h"
#include "poglib/util/assetmanager.h"
#include "poglib/util/glcamera.h"

void ecs_system_render_model(const slot_t *const componentpool, const ecs_componentmanager_t * const cmp_manager)
{
    ASSERT(global_poggen);
    slot_iterator(componentpool, ITER)
    {
        const u32 assetid = ((ecs_component_model_t *)ITER)->asset_id;
        const glmodel_t *model = (glmodel_t *)assetmanager_get_assetresource(
            &global_poggen->systems.assets, 
            ASSET_TYPE_MODEL,
            assetid
        );

        if (!model) {
            continue;
        }

        const gpu_asset_t *gpu_loaded_asset = (gpu_asset_t *)assetmanager_get_gpu_loaded_asset(
            &global_poggen->systems.assets, 
            assetid
        );
        ASSERT(gpu_loaded_asset);

        const u32 entity_id                         = ((ecs_component_model_t *)ITER)->internal.entity_id;
        const ecs_component_material_t *material    = ecs_componentmanager_get_component(cmp_manager, entity_id, ECS_CMP_MATERIAL);
        const ecs_component_transform_t *transform  = ecs_componentmanager_get_component(cmp_manager, entity_id, ECS_CMP_TRANSFORM);
        const glshader_t *shader                    = assetmanager_get_assetresource(&global_poggen->systems.assets, ASSET_TYPE_GLSL_SHADER, material->shader_asset_id);

        ASSERT(material);
        ASSERT(shader);
        ASSERT(transform);

        //TODO: camera ?
        glcamera_t camera = {0};
        matrix4f_t perspective_projection = MATRIX4F_IDENTITY;

        for (u8 idx = 0; idx < model->meshes.len; idx++)
        {
            renderqueue_pass_command(
                &global_poggen->systems.renderqueue, 
                (rendercommand_t) {
                    .command_type = RENDER_COMMAND_TYPE_MESH,
                    .mesh = gpu_loaded_asset->meshes.data[idx],
                    .draw_mode = RENDER_COMMAND_DRAW_MODE_TRIANGLE,
                    .enable_wireframe = false,
                    .instance = {0},
                    .material = {
                        .shader = {
                            .data = shader,
                            .uniforms = {
                                .count = 8,
                                .data = {
                                    [0] = {
                                        .name = str_lit("view"),
                                        .value = glcamera_getview(&camera),
                                    },
                                    [1] = {
                                        .name = str_lit("projection"), 
                                        .value = perspective_projection
                                    },
                                    [2] = {
                                        .name = str_lit("transform"),
                                        .value = glms_translate_make(transform->translation)
                                    },
                                    [3] = {
                                        .name = str_lit("material.color"),
                                        .value.vec4 = *(vec4f_t *)list_get_value(&model->colors, idx)
                                    },
                                    [4] = {
                                        .name = str_lit("uBones"), 
                                        .value.mat4s = {
                                            .count = model->transforms[0].len, 
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


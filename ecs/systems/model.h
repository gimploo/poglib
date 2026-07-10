#pragma once
#include <poglib/poggen.h>
#include "poglib/ecs/common.h"
#include "poglib/ecs/component.h"
#include "poglib/ecs/component/types.h"
#include "poglib/ecs/uniform_registry.h"
#include "poglib/external/cglm/struct/mat4.h"
#include "poglib/gfx/model/assimp.h"
#include "poglib/math/la.h"
#include "poglib/pipeline/render/common.h"
#include "poglib/pipeline/render/render_queue.h"
#include "poglib/poggen.h"
#include "poglib/util/asset.h"
#include "poglib/util/assetmanager.h"
#include "poglib/util/glcamera.h"
#include "poglib/util/workbench/common.h"

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

        uniform_compute_ctx_t uniform_ctx = {
            .active_camera = ctx.active_camera,
            .aspect_ratio = global_engine->handle.app->window.aspect_ratio,
            .transform = transform,
            .is_selected = global_workbench->editor.current_selected_entity_id == entity_id,
        };

        ASSERT(gpu_loaded_asset->meshes.count == model->meshes.len);
        for (u8 idx = 0; idx < model->meshes.len; idx++)
        {
            uniform_ctx.model_color = *(vec4f_t *)list_get_value(&model->colors, idx);
            uniform_ctx.bones.count = model->transforms[idx].len;
            uniform_ctx.bones.data = (matrix4f_t *)model->transforms[idx].data;

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

            gluniforms_t *uniforms = &cmd.material.shader.uniforms;

            hashtable_iterator(&shader->internal.uniformlocs, loc_entry)
            {
                const hashtable_entry_t *he = loc_entry;
                const str_t uniform_name = he->key.str;
                const uniform_binding_t *binding = uniform_registry_lookup(uniform_name);
                if (!binding)
                    eprint("uniform '%.*s' not found in registry for shader '%.*s'", uniform_name.len, uniform_name.data, shader->vs.len, shader->vs.data);

                gluniform_value_t value = uniform_registry_compute(binding->source, &uniform_ctx);

                for (u8 o = 0; o < material->shader.uniforms.count; o++)
                    if (str_cmp(material->shader.uniforms.data[o].name, uniform_name))
                        value = material->shader.uniforms.data[o].value;

                uniforms->data[uniforms->count].name = uniform_name;
                uniforms->data[uniforms->count].value = value;
                uniforms->count++;
            }

            renderqueue_pass_command(&global_engine->systems.renderqueue, cmd);
        }
    }
}

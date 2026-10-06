#pragma once
#include "poglib/ecs/component/types.h"
#include "poglib/ecs/common.h"
#include "poglib/gfx/gl/shader.h"
#include "poglib/poggen.h"
#include <poglib/basic.h>
#include <poglib/math.h>
#include <poglib/util/glcamera.h>
#include "./../component.h"


typedef enum ecs_uniform_supported_types {

    ECS_UNIFORM_CAMERA_PROJECTION       = 0,
    ECS_UNIFORM_CAMERA_VIEW,
    ECS_UNIFORM_CAMERA_POSITION,
    ECS_UNIFORM_TRANSFORM,
    ECS_UNIFORM_MODEL_BONES,
    ECS_UNIFORM_MODEL_JOINTS,
    ECS_UNIFORM_TEXTURE_AVAILABILITY,
    ECS_UNIFORM_LIGHT_POSITION,
    ECS_UNIFORM_SUPPORTED_COUNT,

} ecs_uniform_supported_types;

const str_t ECS_UNIFORM_SUPPORTED_NAME_LOOKUP[ECS_UNIFORM_SUPPORTED_COUNT] = {
    [ECS_UNIFORM_CAMERA_PROJECTION]         = str_lit("projection"),
    [ECS_UNIFORM_CAMERA_VIEW]               = str_lit("view"),
    [ECS_UNIFORM_CAMERA_POSITION]           = str_lit("camerapos"),
    [ECS_UNIFORM_TRANSFORM]                 = str_lit("transform"),
    [ECS_UNIFORM_MODEL_BONES]               = str_lit("uBones"),
    [ECS_UNIFORM_MODEL_JOINTS]              = str_lit("uJoints"),
    [ECS_UNIFORM_TEXTURE_AVAILABILITY]      = str_lit("has_texture"),
    [ECS_UNIFORM_LIGHT_POSITION]            = str_lit("lightPos"),
};


INTERNAL void ecs_system_material__internal__resolve_uniforms(
    ecs_component_material_t *const material,
    const ecs_componentmanager_t *const cmp_manager,
    const u32 entity_id,
    const glcamera_t *const active_camera
) {
    const glshader_t *const shader = assetmanager_get_assetresource(&global_engine->systems.assets, ASSET_TYPE_GLSL_SHADER, material->shader_asset_id);
    if (!shader) return;

    const matrix4f_t projection  = glms_perspective(radians(45), global_engine->handle.app->window.aspect_ratio, 1.0f, 1000.0f);
    const matrix4f_t camera_view = glcamera_getview(active_camera);

    const ecs_entity_query_t view = ecs_componentmanager__internal__query_components(cmp_manager, entity_id, ECS_CMP_TRANSFORM | ECS_CMP_SPRITE | ECS_CMP_SIMULATION | ECS_CMP_MODEL);
    const ecs_component_transform_t *const transform    = view.entity_cmp_data[ECS_CMP_TRANSFORM_IDX];
    ASSERT(transform);
    const ecs_component_sprite_t *const sprite          = view.entity_cmp_data[ECS_CMP_SPRITE_IDX];

    const bool is_entity_simulated = view.entity_cmp_data[ECS_CMP_SIMULATION_IDX] != NULL;

    const matrix4f_t model_transform = !is_entity_simulated ? glms_mat4_mul(
        glms_translate_make(transform->position), 
        glms_mat4_mul(
            glms_quat_mat4(transform->orientation), 
            glms_scale_make(transform->scale)
        )
    ) : MATRIX4F_IDENTITY;

    gluniforms_t uniforms_resolved = {0};
    for (ecs_uniform_supported_types uniform_idx = 0; uniform_idx < ECS_UNIFORM_SUPPORTED_COUNT; uniform_idx++)
    {
        const str_t uniform_name            = ECS_UNIFORM_SUPPORTED_NAME_LOOKUP[uniform_idx];
        const gluniform_meta_t *const meta  = glshader_get_uniform_meta_entry_or_null(shader, uniform_name);
        if (!meta) continue;

        gluniform_value_t value = {0};
        switch(uniform_idx)
        {
            case ECS_UNIFORM_CAMERA_POSITION:
                value.vec3 = active_camera->position;
            break;
            case ECS_UNIFORM_CAMERA_PROJECTION:
                value.mat4 = projection;
            break;
            case ECS_UNIFORM_CAMERA_VIEW:
                value.mat4 = camera_view;
            break;
            case ECS_UNIFORM_TRANSFORM:
                value.mat4 = model_transform;
            break;
            case ECS_UNIFORM_TEXTURE_AVAILABILITY:
                if(view.entity_cmp_data[ECS_CMP_MODEL_IDX]) continue;
                value.f32 = (f32)(material->meshidx_to_texture_lookup.count > 0 || (sprite && sprite->spritesheet_asset_id != INVALID_ASSET_ID));
            break;

            //FIXME: fix this after animations are loaded and understood
            //NOTE: these are handled in model's resolver
            case ECS_UNIFORM_MODEL_JOINTS:
            break;
            case ECS_UNIFORM_MODEL_BONES:
                continue;
            break;

            default: eprint("ecs doesnt suppport given uniform");
        }

        uniforms_resolved.data[uniforms_resolved.count].name    = uniform_name;
        uniforms_resolved.data[uniforms_resolved.count].value   = value;
        uniforms_resolved.count++;
    }

    material->internal.uniform_values = uniforms_resolved;
}

INTERNAL gluniforms_t ecs_material_resolve_per_mesh_uniforms(
    const ecs_component_model_t *const model_cmp,
    ecs_component_material_t *const material_cmp,
    const glshader_t *const shader,
    const u64 mesh_idx
) {
    gluniforms_t out_uniforms = material_cmp->internal.uniform_values;

    if (material_cmp) {
        if (glshader_uniform_is_support(shader, ECS_UNIFORM_SUPPORTED_NAME_LOOKUP[ECS_UNIFORM_TEXTURE_AVAILABILITY])) {
            out_uniforms.data[out_uniforms.count].name       = ECS_UNIFORM_SUPPORTED_NAME_LOOKUP[ECS_UNIFORM_TEXTURE_AVAILABILITY];
            out_uniforms.data[out_uniforms.count].value.f32  = material_cmp->meshidx_to_texture_lookup.data[mesh_idx].basecolor_texture_id != INVALID_ASSET_ID;
            out_uniforms.count++;
        }
    }

    //FIXME: everything below once cgltf integration for animaiton is complete
    if (!model_cmp || !model_cmp->internal.model) return out_uniforms;

    const glmodel_t *const model = model_cmp->internal.model;
    ASSERT(mesh_idx < model->meshes.len);

#if 0
    if (glshader_uniform_is_support(shader, ECS_UNIFORM_SUPPORTED_NAME_LOOKUP[ECS_UNIFORM_MATERIAL_COLOR])) {
        out_uniforms->data[out_uniforms->count].name       = ECS_UNIFORM_SUPPORTED_NAME_LOOKUP[ECS_UNIFORM_MATERIAL_COLOR];
        out_uniforms->data[out_uniforms->count].value.vec4 = *(vec4f_t *)list_get_value(&model->colors, mesh_idx);
        out_uniforms->count++;
    }
#endif

    if (glshader_uniform_is_support(shader, ECS_UNIFORM_SUPPORTED_NAME_LOOKUP[ECS_UNIFORM_MODEL_BONES])) {
        out_uniforms.data[out_uniforms.count].name              = ECS_UNIFORM_SUPPORTED_NAME_LOOKUP[ECS_UNIFORM_MODEL_BONES];
        out_uniforms.data[out_uniforms.count].value.mat4s.count = (u32)model->transforms.data[mesh_idx].len;
        out_uniforms.data[out_uniforms.count].value.mat4s.data  = (matrix4f_t *)model->transforms.data[mesh_idx].data;
        out_uniforms.count++;
    }

    return out_uniforms;
}


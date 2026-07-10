#pragma once
#include "poglib/ecs/component/types.h"
#include "poglib/ecs/common.h"
#include "poglib/gfx/gl/shader.h"
#include "poglib/poggen.h"
#include <poglib/basic.h>
#include <poglib/math.h>
#include <poglib/util/glcamera.h>

INTERNAL void uniform_registry_resolve_material_uniforms(
    ecs_component_material_t *material,
    const ecs_componentmanager_t *cmp_manager,
    u32 entity_id,
    const glcamera_t *active_camera)
{
    const glshader_t *shader = (glshader_t *)assetmanager_get_assetresource(
        &global_engine->systems.assets, ASSET_TYPE_GLSL_SHADER, material->shader.asset_id);
    if (!shader) return;

    const ecs_entity_query_t view = ecs_componentmanager__internal_query_components(
        (ecs_componentmanager_t *)cmp_manager, entity_id, ECS_CMP_TRANSFORM);
    const ecs_component_transform_t *transform = view.entity_cmp_data[ECS_CMP_TRANSFORM_IDX];
    if (!transform) return;

    const matrix4f_t proj = glms_perspective(
        radians(45), global_engine->handle.app->window.aspect_ratio, 1.0f, 1000.0f);
    const matrix4f_t view_mat = glcamera_getview(active_camera);
    const matrix4f_t model_mat = glms_mat4_mul(
        glms_translate_make(transform->position),
        glms_mat4_mul(glms_quat_mat4(transform->orientation), glms_scale_make(transform->scale)));

    hashtable_iterator(&shader->internal.uniformlocs, loc_entry)
    {
        const hashtable_entry_t *he = loc_entry;
        const str_t uniform_name = he->key.str;

        bool already_set = false;
        for (u8 i = 0; i < material->shader.uniforms.count; i++) {
            if (str_cmp(material->shader.uniforms.data[i].name, uniform_name)) {
                already_set = true;
                break;
            }
        }
        if (already_set) continue;

        gluniform_value_t value = {0};
        if (str_cmp(uniform_name, str("view")))
            value.mat4 = view_mat;
        else if (str_cmp(uniform_name, str("projection")))
            value.mat4 = proj;
        else if (str_cmp(uniform_name, str("transform")))
            value.mat4 = model_mat;
        else
            continue;

        ASSERT(material->shader.uniforms.count < MAX_UNIFORMS_ALLOWED_IN_SHADER);
        material->shader.uniforms.data[material->shader.uniforms.count].name = uniform_name;
        material->shader.uniforms.data[material->shader.uniforms.count].value = value;
        material->shader.uniforms.count++;
    }
}

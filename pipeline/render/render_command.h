#pragma once
#include "poglib/gfx/glrenderer2d.h"
#include "poglib/pipeline/render/common.h"
#include "poglib/util/asset.h"
#include <poglib/gfx/glrenderer3d.h>

//NOTE: instance types are to be aligned to 16 bytes for vec3 and vec4 - so better to use vec4
//for all cases, although wasteful - GPU requires to padded that way for those types rest can be
//used as they are.

typedef struct rendercommand_instance_primitive_mesh_t rendercommand_instance_primitive_mesh_t;
struct rendercommand_instance_primitive_mesh_t {
    box_t uv;
    vec4f_t color;
    vec4f_t translation;
    vec4f_t orientation;
    vec4f_t scale;
};

typedef struct rendercommand_instance_line_t rendercommand_instance_line_t;
struct rendercommand_instance_line_t {
    vec4f_t color;
    vec4f_t translation;
    vec4f_t orientation;
    vec4f_t scale;
};

typedef struct rendercommand_t rendercommand_t;
struct rendercommand_t {

    gpu_mesh_t *const mesh;
    struct {
        struct {
            gluniforms_t uniforms;
            const glshader_t *data;
        } shader;
        struct {
            u16 count;
            u16 ids[MAX_TEXTURES_ALLOWED_PER_RENDER];
        } textures;
    } material;

    buffer_t                    instance;
    rendercommand_draw_mode     draw_mode;
    bool                        enable_wireframe;
};

#ifndef IGNORE_RENDER_COMMAND_IMPLEMENTATION

bool rendercommand__internal_compare_textures_ids(const rendercommand_t *const rc1, const rendercommand_t *const rc2)
{
    ASSERT(rc1 && rc2);

    if (rc1->material.textures.count != rc2->material.textures.count) 
        return false;

    for (u8 tex_idx = 0; tex_idx < rc1->material.textures.count; tex_idx++)
        if(rc1->material.textures.ids[tex_idx] != rc2->material.textures.ids[tex_idx])
            return false;

    return true;

}

bool rendercommand__internal_compare_shader_and_uniforms(const rendercommand_t * const bucket_rc1, const rendercommand_t * const rc2)
{
    ASSERT(bucket_rc1 && rc2);

    if (bucket_rc1->material.shader.data == NULL && rc2->material.shader.data == NULL)
        return true;

    if (bucket_rc1->material.shader.data && rc2->material.shader.data == NULL)
        return false;

    if (bucket_rc1->material.shader.data == NULL && rc2->material.shader.data)
        return false;

    if (bucket_rc1->material.shader.data->id != rc2->material.shader.data->id)
        return false;

    if (bucket_rc1->material.shader.uniforms.count != rc2->material.shader.uniforms.count)
        return false;

    for(u8 uniform_idx = 0; uniform_idx < bucket_rc1->material.shader.uniforms.count; uniform_idx++)
        if (!str_cmp(bucket_rc1->material.shader.uniforms.data[uniform_idx].name, rc2->material.shader.uniforms.data[uniform_idx].name)) 
            return false;

    return true;
}

#endif

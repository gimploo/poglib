#pragma once
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

typedef struct rendercommand_primitive_line_t rendercommand_primitive_line_t;
struct rendercommand_primitive_line_t {
    vec3f_t start;
    vec3f_t end;
    vec4f_t startcolor;
    vec4f_t endcolor;
};

typedef struct rendercommand_primitive_triangle_t rendercommand_primitive_triangle_t;
struct rendercommand_primitive_triangle_t {
    vec3f_t points[3];
    vec4f_t color;
};

typedef struct rendercommand_t rendercommand_t;
struct rendercommand_t {

    struct {
        enum {
            RENDERCOMMAND_VTX_TYPE_MESH         = 0,
            RENDERCOMMAND_VTX_TYPE_LINE         = 1,
            RENDERCOMMAND_VTX_TYPE_TRIANGLES    = 2,
        } type;
        union {
            rendercommand_primitive_triangle_t  triangle;
            rendercommand_primitive_line_t      line;
            gpu_mesh_t                          *mesh;
        } data;
    } vtx;

    struct {
        struct {
            gluniforms_t    uniforms;
            glshader_t      *data;
        } shader;
        gltexturelist_t texture;
    } material;

    buffer_t    instance;
    bool        enable_wireframe;
};

#ifndef IGNORE_RENDER_COMMAND_IMPLEMENTATION

bool rendercommand__internal_compare_textures_ids(const rendercommand_t *const rc1, const rendercommand_t *const rc2)
{
    ASSERT(rc1 && rc2);

    if (rc1->material.texture.count != rc2->material.texture.count)
        return false;

    for (u8 tex_idx = 0; tex_idx < rc1->material.texture.count; tex_idx++) {
        const gltextureitem_t *a = &rc1->material.texture.items[tex_idx];
        const gltextureitem_t *b = &rc2->material.texture.items[tex_idx];
        if (a->type != b->type) return false;
        if (a->source.normal_texture->id != b->source.normal_texture->id) return false;
    }

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

#pragma once
#include <GL/glew.h>
#include "gl/shader.h"
#include "gl/texture2d.h"
#include "gl/objects.h"
#include "gl/framebuffer.h"
#include "gl/types.h"
#include "model/assimp.h"

/*=============================================================================
                        - OPENGL 2D RENDERER -
=============================================================================*/

typedef struct glrenderer3d_t {

    const glshader_t    *shader;
    struct {
        gltexture2d_t   *data;
        int             top;
    } textures;

} glrenderer3d_t ;

typedef struct {
    glshader_t *shader;
    struct {
        u8 count;
        struct {
            const char *name;
            const char *type;
            union {
                matrix4f_t  mat4;
                vec4f_t     vec4;
                vec3f_t     vec3;
                vec2f_t     vec2;
            } value;
        } uniform[10];
    } uniforms;
} glshaderconfig_t;

typedef struct {

    u32 count;
    glshaderconfig_t configs[3];

} glshaderconfiglist_t;

typedef struct {

    // Vertex data
    struct {
        u32 size;
        u8 *data;
    } vtx;

    // Index data
    struct {
        u8 *data;
        u32 nmemb;
    } idx;

    // Attributes
    struct {
        u8 count;
        struct {
            u8 ncmp;
            struct {
                u32 offset;
                u32 stride;
            } interleaved;
        } attr[10];
    } attrs;

    // Textures
    struct {
        u8 count;
        gltexture2d_t **textures;
    } textures;

    // Shader Config { uniform and shader }
    glshaderconfig_t shader_config;

} glrendercall_t;

typedef struct {

    //NOTE: we are in the assumption that only float values are allowed via attributes
    //other types are not supported 

    // Draw Call
    struct {
        const u8 count;
        const glrendercall_t call[10];
    } calls;

} glrendererconfig_t;



void                glrenderer3d_draw_cube(const glrenderer3d_t *renderer);
void                glrenderer3d_draw_model(const glmodel_t *, const glshaderconfiglist_t);
void                glrenderer3d_draw(const glrendererconfig_t config);


/*-----------------------------------------------------------------------------
                            IMPLEMENTATION
-----------------------------------------------------------------------------*/

#ifndef IGNORE_GLRENDERER2D_IMPLEMENTATION

void glrenderer3d_draw_cube(const glrenderer3d_t *self)
{
    const f32 vertices[] = {
        // vertex             // uv        // normals
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  0.0f, 0.0f, -1.0f,   
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,  0.0f, 0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  0.0f, 0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  0.0f, 0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  0.0f, 0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  0.0f, 0.0f, -1.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  0.0f, 0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,  0.0f, 0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,  0.0f, 0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,  0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,  0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  0.0f, 0.0f, 1.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  -1.0f, 0.0f, 0.0f,   
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  -1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  -1.0f, 0.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  1.0f, 0.0f, 0.0f, 
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  1.0f, 0.0f, 0.0f, 
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  1.0f, 0.0f, 0.0f, 
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  1.0f, 0.0f, 0.0f, 
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  1.0f, 0.0f, 0.0f, 
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  1.0f, 0.0f, 0.0f, 

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  0.0f, -1.0f, 0.0f, 
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,  0.0f, -1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,  0.0f, -1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,  0.0f, -1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  0.0f, -1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  0.0f, -1.0f, 0.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  0.0f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  0.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  0.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,  0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  0.0f, 1.0f, 0.0f
    };

    unsigned int VBO, VAO;
    GL_CHECK(glGenVertexArrays(1, &VAO));
    GL_CHECK(glGenBuffers(1, &VBO));
    GL_CHECK(glBindVertexArray(VAO));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, VBO));
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW));
    GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0));
    GL_CHECK(glEnableVertexAttribArray(0));
    GL_CHECK(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))));
    GL_CHECK(glEnableVertexAttribArray(1));
    GL_CHECK(glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float))));
    GL_CHECK(glEnableVertexAttribArray(2));

    glshader_bind(self->shader);
    if (self->textures.data && self->textures.top > 0)
        for (int i = 0; i < self->textures.top; i++)
            gltexture2d_bind(&self->textures.data[i], i);

    GL_CHECK(glBindVertexArray(VAO));
    GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, 36));

    GL_CHECK(glDeleteVertexArrays(1, &VAO));
    GL_CHECK(glDeleteBuffers(1, &VBO));
}

void glrenderer3d_draw_model(const glmodel_t *model, const glshaderconfiglist_t config)
{
    ASSERT(model->meshes.len > 0);
    if(model->meshes.len > 3) 
        eprint("TODO: Kept a hard limit on how many meshes (3) can be rendered per model");
    ASSERT(config.count == model->meshes.len);

    glrendercall_t calls[3] = {0};

    list_iterator(&model->meshes, iter) 
    {
        glmesh_t *mesh = iter;
        calls[(u64)list_index] = (glrendercall_t ){

            .textures = {
                .count = model->textures.len,
                .textures = (gltexture2d_t **)list_get_buffer(&model->textures)
            },

            .attrs = {
                .count = 3,
                .attr = {
                    [0] = {
                        .ncmp = 3,
                        .interleaved = {
                            .offset = 0,
                            .stride = sizeof(glvertex3d_t) ,
                        }
                    },
                    [1] = {
                        .ncmp = 3,
                        .interleaved = {
                            .offset = offsetof(glvertex3d_t, norm),
                            .stride = sizeof(glvertex3d_t),
                        }
                    },
                    [2] = {
                        .ncmp = 2,
                        .interleaved = {
                            .offset = offsetof(glvertex3d_t, uv),
                            .stride = sizeof(glvertex3d_t)
                        }
                    }
                }
            },
            .shader_config = config.configs[(u64)list_index],
            .vtx = {
                .data = slot_get_buffer(&mesh->vtx),
                .size = slot_get_size(&mesh->vtx)
            },
            .idx = {
                .data = slot_get_buffer(&mesh->idx),
                .nmemb = mesh->idx.len
            },
        };
    }

    glrenderer3d_draw((glrendererconfig_t) {
        .calls = {
            .count = model->meshes.len, 
            .call = {
                [0] = calls[0],
                [1] = calls[1],
                [2] = calls[2]
            }
        }
    });
}


//NOTE: we have all the buffers combined to one single vbo and this creates a concern
//of whether can the vbo be big enough to store all of it - needs more testing
void glrenderer3d_draw(const glrendererconfig_t config)
{
    ASSERT(config.calls.count > 0);
    for (u8 call_idx = 0; call_idx < config.calls.count; call_idx++)
    {
        bool is_idx_null = config.calls.call[call_idx].idx.data ? false : true;

        vao_t vao = vao_init();
        vao_bind(&vao);

        vbo_t vbo = {0};
        ebo_t ebo = {0};

        // Vertex and Index buffer init
        ASSERT(config.calls.call[call_idx].vtx.data);
        ASSERT(config.calls.call[call_idx].vtx.size > 0); 
        if (!is_idx_null) {

            ASSERT(config.calls.call[call_idx].idx.nmemb > 0); 

            vbo = vbo_static_init(
                    config.calls.call[call_idx].vtx.data, 
                    config.calls.call[call_idx].vtx.size, 
                    config.calls.call[call_idx].idx.nmemb);
            vbo_bind(&vbo);
            ebo = ebo_init(
                    &vbo, 
                    (u32 *)config.calls.call[call_idx].idx.data, 
                    config.calls.call[call_idx].idx.nmemb);
            ebo_bind(&ebo);

        } else {
            u32 total_ncmp = 0;
            ASSERT(config.calls.call[call_idx].attrs.count > 0);
            for(u8 attr_idx = 0; 
                    attr_idx < config.calls.call[call_idx].attrs.count; ++attr_idx)
                total_ncmp += config.calls.call[call_idx].attrs.attr[attr_idx].ncmp;

            vbo = vbo_static_init(
                    config.calls.call[call_idx].vtx.data, 
                    config.calls.call[call_idx].vtx.size, 
                    config.calls.call[call_idx].vtx.size / (total_ncmp * sizeof(f32)));
            vbo_bind(&vbo);
        }

        //Attributes
        for(u8 attr_idx = 0; attr_idx < config.calls.call[call_idx].attrs.count; ++attr_idx)
        {
            vao_set_attributes(
                    &vao, 
                    &vbo, 
                    config.calls.call[call_idx].attrs.attr[attr_idx].ncmp, 
                    GL_FLOAT, 
                    false, 
                    config.calls.call[call_idx].attrs.attr[attr_idx].interleaved.stride, 
                    config.calls.call[call_idx].attrs.attr[attr_idx].interleaved.offset); 

        }

        // Shader
        ASSERT(config.calls.call[call_idx].shader_config.shader);
        glshader_bind(config.calls.call[call_idx].shader_config.shader); 

        //uniforms
        ASSERT(config.calls.call[call_idx].shader_config.uniforms.count >= 0);
        for (u8 uni_idx = 0; uni_idx < config.calls.call[call_idx].shader_config.uniforms.count; uni_idx++)
        {
            const struct {
                const char *name;
                const char *type;
                union {
                    matrix4f_t  mat4;
                    vec4f_t     vec4;
                    vec3f_t     vec3;
                    vec2f_t     vec2;
                } value;
            } *uniform = &config.calls.call[call_idx].shader_config.uniforms.uniform[uni_idx];

            if (strcmp(uniform->type, "matrix4f_t") == 0)
                glshader_send_uniform_matrix4f(
                        config.calls.call[call_idx].shader_config.shader, 
                        uniform->name, 
                        uniform->value.mat4);
            else if (strcmp(uniform->type, "vec4f_t" ) == 0) 
                glshader_send_uniform_vec4f(
                        config.calls.call[call_idx].shader_config.shader, 
                        uniform->name, 
                        uniform->value.vec4);
            else if (strcmp(uniform->type, "vec3f_t" ) == 0)
                glshader_send_uniform_vec3f(
                        config.calls.call[call_idx].shader_config.shader, 
                        uniform->name, 
                        uniform->value.vec3);
            else if (strcmp(uniform->type, "vec2f_t" ) == 0)
                glshader_send_uniform_vec2f(
                        config.calls.call[call_idx].shader_config.shader, 
                        uniform->name, 
                        uniform->value.vec2);
            else eprint("unknown uniform type `%s` for name `%s`", 
                    uniform->type, uniform->name);
        }

        //Textures
        for (u8 txt_idx = 0; txt_idx < config.calls.call[call_idx].textures.count; ++txt_idx)
        {
            gltexture2d_bind(
                    config.calls.call[call_idx].textures.textures[txt_idx],
                    txt_idx);
        }

        if (!is_idx_null)   vao_draw_with_ebo(&vao, &ebo);
        else                vao_draw_with_vbo(&vao, &vbo);

        gltexture2d_unbind();

        if (!is_idx_null) ebo_destroy(&ebo);
        vao_destroy(&vao);
        vbo_destroy(&vbo);

    }

}
#endif


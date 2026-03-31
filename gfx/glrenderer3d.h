#pragma once
#include "gl/framebuffer.h"
#include "gl/types.h"
#include "model/assimp.h"
#include "poglib/basic/ds/list.h"
#include "poglib/gfx/gl/common.h"
#include "poglib/gfx/gl/material.h"
#include "poglib/gfx/gl/texture2d.h"
#include "poglib/gfx/gl/renderconfig.h"
#include "poglib/gfx/gl/vbo_stream_types.h"

//NOTE: Attributes are only handelled for GL_FLOAT (default) and GL_INT


/*=============================================================================
                        - OPENGL 3D RENDERER -
=============================================================================*/

#define MAX_DRAW_CALLS_PER_FRAME_COUNT 100

typedef struct glrenderer3d_t {

    const glshader_t    *shader;
    struct {
        gltexture2d_t   *data;
        int             top;
    } textures;

} glrenderer3d_t ;

typedef struct {

    // Draw type
    enum {
        LINE = GL_LINE,
        TRIANGLES = GL_TRIANGLES, //default
    } draw_mode;

    bool is_wireframe; //default false
    bool allow_empty_vtx_buffer; //default to false
    bool disable_depth_buffer;
    struct {
        bool enable;
        u32 count;
    } instancing;

    // Vertex data
    buffer_t vtx[VBO_STREAM_TYPE_COUNT];

    // Index data
    struct {
        u8 *data;
        u32 nmemb;
    } idx;

    // Attributes
    struct {
        u8 count;
        glvtx_attribute_t attr[16];
    } attrs;

    // Textures
    gltexturelist_t textures;

    // Shader Config { uniform and shader }
    glshaderconfig_t shader_config;

} glrendercall_t;

typedef struct {

    //NOTE: we are in the assumption that only float values are allowed via attributes
    //other types are not supported 

    // Draw Call
    struct {
        u8 count;
        glrendercall_t call[MAX_DRAW_CALLS_PER_FRAME_COUNT];
    } calls;

} glrendererconfig_t;


void                glrenderer3d_draw_cube(const glrenderer3d_t *renderer);
void                glrenderer3d_draw_model(const glmodel_t *model, const glshaderconfiglist_t config, bool in_wireframe);
void                glrenderer3d_drawcall(const glrendercall_t call);
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

void glrenderer3d_draw_model(const glmodel_t *model, const glshaderconfiglist_t config, bool in_wireframe)
{
    ASSERT(model->meshes.len > 0);
    if(model->meshes.len > MAX_DRAW_CALLS_PER_FRAME_COUNT) 
        eprint("FIXME: Kept a hard limit on how many meshes (100) can be rendered per model");

    bool common_shader = false;
    if (config.count == 1) {
        common_shader = true;
    } else {
        ASSERT(config.count == model->meshes.len);
    }

    glrendererconfig_t renderconfig = {
        .calls = {
            .count = model->meshes.len,
            .call = {0}
        }
    };

    const gltexturelist_t textures = glmodel_get_texuturelist(model);

    list_iterator(&model->meshes, iter) 
    {
        glmesh_t *mesh = iter;
        renderconfig.calls.call[(u64)list_index] = (glrendercall_t ){
            .is_wireframe = in_wireframe,
            .textures = textures,
            .attrs = {
                .count = 7,
                .attr = {
                    [0] = {
                        .ncmp = 3,
                        .type = GL_FLOAT,
                        .interleaved = {
                            .offset = 0,
                            .stride = sizeof(glvertex3d_t) ,
                        }
                    },
                    [1] = {
                        .ncmp = 3,
                        .type = GL_FLOAT,
                        .interleaved = {
                            .offset = offsetof(glvertex3d_t, norm),
                            .stride = sizeof(glvertex3d_t),
                        }
                    },

                    [2] = {
                        .ncmp = 2,
                        .type = GL_FLOAT,
                        .interleaved = {
                            .offset = offsetof(glvertex3d_t, uv),
                            .stride = sizeof(glvertex3d_t)
                        }
                    },
                    [3] = {
                        .ncmp = 3,
                        .type = GL_FLOAT,
                        .interleaved = {
                            .offset = offsetof(glvertex3d_t, tangents),
                            .stride = sizeof(glvertex3d_t)
                        }
                    },
                    [4] = {
                        .ncmp = 3,
                        .type = GL_FLOAT,
                        .interleaved = {
                            .offset = offsetof(glvertex3d_t, bitangents),
                            .stride = sizeof(glvertex3d_t)
                        }
                    },
                    [5] = {
                        .ncmp = 4,
                        .type = GL_INT,
                        .interleaved = {
                            .offset = offsetof(glvertex3d_t, bone_ids),
                            .stride = sizeof(glvertex3d_t)
                        }
                    },
                    [6] = {
                        .ncmp = 4,
                        .type = GL_FLOAT,
                        .interleaved = {
                            .offset = offsetof(glvertex3d_t, bone_weights),
                            .stride = sizeof(glvertex3d_t)
                        }
                    }
                }
            },
            .shader_config = common_shader ? config.configs[0] : config.configs[(u64)list_index],
            .vtx = {
                [VBO_STREAM_TYPE_GEOMETRY] = {
                    .raw_data = slot_get_buffer(&mesh->vtx),
                    .size = slot_get_size(&mesh->vtx)
                }
            },
            .idx = {
                .data = slot_get_buffer(&mesh->idx),
                .nmemb = mesh->idx.len
            },
        };
    }

    glrenderer3d_draw(renderconfig);
}

void glrenderer3d_drawcall(const glrendercall_t call)
{
    glrenderer3d_draw((glrendererconfig_t){
       .calls = {
            .count = 1,
            .call = {
                [0] = call
            }
       }
    });
}


//TODO: here we are reallocating memory inside the GPU every loop - check vbo logic.
//Explore strategies to pre allocate static memory and a fixed dynamic memory and maybe stream
//the dyanmic stuff into the pre allocated dynaimc mem
void glrenderer3d_draw(const glrendererconfig_t config)
{
    ASSERT(config.calls.count > 0);
    for (u8 call_idx = 0; call_idx < config.calls.count; call_idx++)
    {
        bool is_idx_null = config.calls.call[call_idx].idx.data ? false : true;
        const bool enable_instancing = config.calls.call[call_idx].instancing.enable;

        //Defaults to GL_TRIANGES if not set else to whatever modes that is available
        u8 draw_mode = GL_TRIANGLES;
        if (config.calls.call[call_idx].draw_mode) {
            draw_mode = config.calls.call[call_idx].draw_mode;
        }

        vao_t vao = vao_init();
        vao_bind(&vao);

        vbo_t vbo = {0};
        ebo_t ebo = {0};

        // Vertex and Index buffer init
        if (config.calls.call[call_idx].allow_empty_vtx_buffer && !config.calls.call[call_idx].vtx[VBO_STREAM_TYPE_GEOMETRY].size) 
            return;
        else ASSERT(config.calls.call[call_idx].vtx[VBO_STREAM_TYPE_GEOMETRY].size > 0);

        ASSERT(config.calls.call[call_idx].vtx[VBO_STREAM_TYPE_GEOMETRY].raw_data);
        if (!is_idx_null) {

            ASSERT(config.calls.call[call_idx].idx.nmemb > 0); 

            vbo = vbo_init((vbo_config_t) {
                .usage = GL_STATIC_DRAW,
                .chunks = {
                    [VBO_STREAM_TYPE_GEOMETRY] = {
                        .vertex_count = config.calls.call[call_idx].idx.nmemb, 
                        .buffer = config.calls.call[call_idx].vtx[VBO_STREAM_TYPE_GEOMETRY]
                    },
                    [VBO_STREAM_TYPE_INSTANCE] = enable_instancing ? (vbo_stream_t){ 
                        .instance_count = config.calls.call[call_idx].instancing.count, 
                        .buffer = config.calls.call[call_idx].vtx[VBO_STREAM_TYPE_INSTANCE]
                    } : (vbo_stream_t){0},
                }
            });
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

            const u32 total_geometry_count = config.calls.call[call_idx].vtx[VBO_STREAM_TYPE_GEOMETRY].size / (total_ncmp * sizeof(f32));

            vbo = vbo_init((vbo_config_t) {
                .usage = GL_STATIC_DRAW,
                .chunks = {
                    [VBO_STREAM_TYPE_GEOMETRY] = {
                        .vertex_count = total_geometry_count, 
                        .buffer = config.calls.call[call_idx].vtx[VBO_STREAM_TYPE_GEOMETRY]
                    },
                    [VBO_STREAM_TYPE_INSTANCE] = enable_instancing ? (vbo_stream_t){ 
                        .instance_count = config.calls.call[call_idx].instancing.count, 
                        .buffer = config.calls.call[call_idx].vtx[VBO_STREAM_TYPE_INSTANCE]
                    } : (vbo_stream_t){0},
                }
            });

            vbo_bind(&vbo);
        }

        //Attributes
        for(u32 attr_idx = 0; attr_idx < config.calls.call[call_idx].attrs.count; ++attr_idx)
        {
            const u32 data_type = config.calls.call[call_idx].attrs.attr[attr_idx].type;

            vao_set_attributes(
                &vao,
                &vbo, 
                config.calls.call[call_idx].attrs.attr[attr_idx].ncmp, 
                data_type == GL_INT ? GL_INT : GL_FLOAT,
                false, 
                config.calls.call[call_idx].attrs.attr[attr_idx].interleaved.stride, 
                config.calls.call[call_idx].attrs.attr[attr_idx].interleaved.offset,
                config.calls.call[call_idx].attrs.attr[attr_idx].vbo_chunk_index == VBO_STREAM_TYPE_INSTANCE,
                config.calls.call[call_idx].attrs.attr[attr_idx].vbo_chunk_index); 
        }

        // Shader
        ASSERT(config.calls.call[call_idx].shader_config.shader);
        glshader_bind(config.calls.call[call_idx].shader_config.shader); 

        //uniforms
        ASSERT(config.calls.call[call_idx].shader_config.uniforms.count >= 0);
        for (u8 uni_idx = 0; uni_idx < config.calls.call[call_idx].shader_config.uniforms.count; uni_idx++)
        {
            uniform_t *uniform = (void *)&config.calls.call[call_idx].shader_config.uniforms.uniform[uni_idx];

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
            else if (strcmp(uniform->type, "matrix4f_t []") == 0)
                glshader_send_uniform_matrix4fv(
                        config.calls.call[call_idx].shader_config.shader, 
                        uniform->name,
                        uniform->value.mat4s.data,
                        uniform->value.mat4s.count);
            else if (strcmp(uniform->type, "i32") == 0)
                glshader_send_uniform_ival(
                        config.calls.call[call_idx].shader_config.shader, 
                        uniform->name,
                        uniform->value.i32);
            else if (strcmp(uniform->type, "f32") == 0)
                glshader_send_uniform_fval(
                        config.calls.call[call_idx].shader_config.shader, 
                        uniform->name,
                        uniform->value.f32);
            else if (strcmp(uniform->type, "u32") == 0)
                glshader_send_uniform_uival(
                        config.calls.call[call_idx].shader_config.shader, 
                        uniform->name,
                        uniform->value.u32);
            else if (strcmp(uniform->type, "boolean") == 0)
                glshader_send_uniform_ival(
                        config.calls.call[call_idx].shader_config.shader, 
                        uniform->name,
                        uniform->value.boolean);
            else eprint("unknown uniform type `%s` for name `%s`", 
                    uniform->type, uniform->name);
        }

        //Textures
        for (u8 txt_idx = 0; txt_idx < config.calls.call[call_idx].textures.count; ++txt_idx)
        {
            switch(config.calls.call[call_idx].textures.items[txt_idx].type)
            {
                case GL_TEXTURE_TYPE_NORMAL:
                    gltexture2d_bind(
                        config.calls.call[call_idx].textures.items[txt_idx].source.normal_texture,
                        txt_idx
                    );
                break;
                case GL_TEXTURE_TYPE_CUBEMAP:
                    GL_CHECK(glDepthMask(false));
                    GL_CHECK(glDepthFunc(GL_LEQUAL));
                    glcubemap_bind(
                        config.calls.call[call_idx].textures.items[txt_idx].source.cubemap
                    );
                break;
                default: eprint("Not implemented");
            }
        }

        if (config.calls.call[call_idx].is_wireframe) {
            GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
        } else {
            GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
        }

        if (!config.calls.call[call_idx].disable_depth_buffer) {
            GL_CHECK(glEnable(GL_DEPTH_TEST));
        }

        if (enable_instancing) {
            if (!is_idx_null)   vao_draw_with_ebo_in_mode_instanced(&vao, &ebo, draw_mode);
            else                vao_draw_with_vbo_in_mode_instanced(&vao, &vbo, draw_mode);
        } else {
            if (!is_idx_null)   vao_draw_with_ebo(&vao, &ebo);
            else                vao_draw_with_vbo_in_mode(&vao, &vbo, draw_mode);
        }

        GL_CHECK(glDepthMask(true));
        gltexture2d_unbind();

        if (!is_idx_null) ebo_destroy(&ebo);
        vao_destroy(&vao);
        vbo_destroy(&vbo);

        GL_CHECK(glDepthFunc(GL_LESS));

        if (!config.calls.call[call_idx].disable_depth_buffer) {
            GL_CHECK(glClear(GL_DEPTH_BUFFER_BIT));
        }

    }

}
#endif

